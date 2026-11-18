#include "../../../../Libs/ImGui/imgui.h"
#include "../../../../Manager/ResourceManager.h"
#include "../../../../Manager/SceneManager.h"
#include "../../../../Manager/CollisionManager.h"
#include "../../../../Manager/Camera.h"
#include "../../../../Manager/InputManager.h"
#include "../../../../Manager/SoundManager.h"
#include "../../../../Manager/EditorManager.h"
#include "../../../../Utility/AsoUtility.h"
#include "../../../Common/Collider/ColliderLine.h"
#include "../../../Common/Collider/ColliderCapsule.h"
#include "../../Weapon/WeaponBase.h"
#include "../../Weapon/WeaponManager.h"
#include "State/BossIdleState.h"
#include "State/BossAttackState.h"
#include "State/BossChaseState.h"
#include "State/BossMoveState.h"
#include "EnemyBoss.h"

namespace
{
	constexpr const wchar_t* MODEL_ENEMY_BOSS_NAME = L"ModelEnemyBoss";
	constexpr const wchar_t* JSON_ENEMY_DATA_NAME = L"EnemyData";
	constexpr const wchar_t* ANIM_BOOS_MOVE = L"AnimationBossMove";
	constexpr const wchar_t* ANIM_BOOS_ATTACK = L"AnimationBossAttack";
}

EnemyBoss::EnemyBoss(void)
	: EnemyBase()
	, state_(static_cast<int>(EnemyBoss::STATE::MOVE))
{
}

EnemyBoss::~EnemyBoss(void)
{
}

Json& EnemyBoss::GetData(void) const
{
	return resMng_.GetJson(JSON_ENEMY_DATA_NAME);
}

void EnemyBoss::Release(void)
{
#ifdef _DEBUG
	EditorManager::GetInstance().UnregisterObject("EnemyBoss");
#endif
	CharactorBase::Release();
}

void EnemyBoss::InitLoad(void)
{
	transform_.SetModel(resMng_.LoadModelDuplicate(MODEL_ENEMY_BOSS_NAME));

	// 武器登録
	weapons_->Add(WeaponBase::WEAPON_TYPE::SWORD, transform_, 49);
	weapons_->Add(WeaponBase::WEAPON_TYPE::SHIELD, transform_, 21);
	weapons_->Load();
}

void EnemyBoss::InitTransform(void)
{
	const auto& trans = GetData()["Transform"];
	transform_.pos = trans["Position"];
	transform_.scl = VScale(AsoUtility::VECTOR_ONE, trans["Scale"]);
	transform_.quaRot = Quaternion::Identity();
	transform_.quaRotLocal = Quaternion::AngleAxis(
		AsoUtility::Deg2RadF(trans["Rotation"]["y"].get<float>()), AsoUtility::AXIS_Y);
	transform_.Update();
}

void EnemyBoss::InitCollider(void)
{
	const auto& colData = GetData()["collider"];

	// 線分コライダ
	std::vector<ColliderBase::TAG> lineTags = { ColliderBase::TAG::STAGE };
	auto colLine = std::make_shared<ColliderLine>(
		ColliderBase::TAG::GROUND,
		lineTags,
		&transform_,
		colData["line"]["startPos"],
		colData["line"]["endPos"]);
	ownColliders_[static_cast<int>(ColliderBase::SHAPE::LINE)].push_back(colLine);
	colMng_.AddCollider(colLine);

	// カプセルコライダ
	std::vector<ColliderBase::TAG> capsuleTags = {
		ColliderBase::TAG::PLAYER,
		ColliderBase::TAG::PLAYER_WEPON,
		ColliderBase::TAG::STAGE
	};
	auto colCapsule = std::make_shared<ColliderCapsule>(
		ColliderBase::TAG::ENEMY,
		capsuleTags,
		&transform_,
		colData["capsule"]["topPos"],
		colData["capsule"]["downPos"],
		colData["capsule"]["radius"].get<float>());
	ownColliders_[static_cast<int>(ColliderBase::SHAPE::CAPSULE)].push_back(colCapsule);
	colMng_.AddCollider(colCapsule);
}

void EnemyBoss::InitAnimation(void)
{
	animationController_ = std::make_unique<AnimationController>(transform_.modelId);
	const auto& anim = GetData()["animationSpeed"];

	animationController_->Add(static_cast<int>(ANIM_TYPE::WALK),
		anim["walk"], resMng_.LoadModelDuplicate(ANIM_BOOS_MOVE));
	animationController_->Add(static_cast<int>(ANIM_TYPE::RUN),
		anim["run"], resMng_.LoadModelDuplicate(ANIM_BOOS_MOVE));
	animationController_->Add(static_cast<int>(ANIM_TYPE::ATTACK),
		anim["attack"], resMng_.LoadModelDuplicate(ANIM_BOOS_ATTACK));

	animationController_->Play(static_cast<int>(ANIM_TYPE::WALK));
}

void EnemyBoss::InitPost(void)
{
	weapons_->LoadEnd();

	InitState();

	SetBlackboard();

#ifdef _DEBUG
	EditorManager::GetInstance().RegisterObject({
		"EnemyBoss",
		JSON_ENEMY_DATA_NAME,
		[this]() { this->ApplyJson(); },
		[this]() { this->UpdateGui(); }
		});
#endif
}

void EnemyBoss::ApplyJson(void)
{
	const auto& data = GetData();

	// Transform反映
	transform_.scl = transform_.scl = VScale(AsoUtility::VECTOR_ONE, data["Scale"]);
	transform_.quaRotLocal = Quaternion::AngleAxis(
		AsoUtility::Deg2RadF(data["Transform"]["Rotation"]["y"].get<float>()), AsoUtility::AXIS_Y);
	transform_.Update();

	// コライダー反映
	const auto& col = data["collider"];
	for (auto& c : ownColliders_[static_cast<int>(ColliderBase::SHAPE::LINE)])
	{
		if (auto line = std::dynamic_pointer_cast<ColliderLine>(c))
		{
			line->SetLocalPosStart(col["line"]["startPos"]);
			line->SetLocalPosEnd(col["line"]["endPos"]);
		}
	}
	for (auto& c : ownColliders_[static_cast<int>(ColliderBase::SHAPE::CAPSULE)])
	{
		if (auto cap = std::dynamic_pointer_cast<ColliderCapsule>(c))
		{
			cap->SetLocalPosTop(col["capsule"]["topPos"]);
			cap->SetLocalPosDown(col["capsule"]["downPos"]);
			cap->SetRadius(col["capsule"]["radius"].get<float>());
		}
	}

	// アニメーション速度反映
	if (animationController_)
	{
		const auto& anim = data["animationSpeed"];
		animationController_->SetSpeed(static_cast<int>(ANIM_TYPE::IDLE), anim["idle"]);
		animationController_->SetSpeed(static_cast<int>(ANIM_TYPE::WALK), anim["walk"]);
		animationController_->SetSpeed(static_cast<int>(ANIM_TYPE::RUN), anim["run"]);
		animationController_->SetSpeed(static_cast<int>(ANIM_TYPE::ATTACK), anim["attack"]);
	}
}

void EnemyBoss::UpdateGui(void)
{
#ifdef _DEBUG
	if (ImGui::CollapsingHeader("Runtime State", ImGuiTreeNodeFlags_DefaultOpen))
	{
		if (ImGui::DragFloat3("Current Pos", &transform_.pos.x, 1.0f)) transform_.Update();
		if (ImGui::Button("Warp to Init Pos"))
		{
			transform_.pos = GetData()["Transform"]["Position"];
			transform_.Update();
		}
		ImGui::SameLine();
		if (ImGui::Button("Set Current Pos -> JSON"))
		{
			GetData()["Transform"]["Position"] = transform_.pos;
		}
	}
#endif
}

void EnemyBoss::UpdateProcess(void)
{
	SetBlackboard();

	currentState_->Update(*this);
}

void EnemyBoss::UpdateProcessPost(void)
{
}

void EnemyBoss::UpdateHitCollider(void)
{
}

void EnemyBoss::InitState(void)
{
	states_.clear();
	states_.emplace(static_cast<int>(STATE::IDLE), std::make_unique<BossIdleState>());
	states_.emplace(static_cast<int>(STATE::MOVE), std::make_unique<BossMoveState>());
	states_.emplace(static_cast<int>(STATE::ATTACK), std::make_unique<BossAttackState>());

	currentState_ = states_.at(state_).get();
	currentState_->StartState(*this);
}

void EnemyBoss::SetBlackboard(void)
{
	blackboard_.angleToPlayer = AsoUtility::GetAconf(*targetPos_, transform_.pos, transform_.GetForward(), 50.0f);
	blackboard_.distance = AsoUtility::GetAppropriateDistance(*targetPos_, transform_.pos, 1500.0f);
}

void EnemyBoss::ChangeState(int state)
{
	if (state_ == state) return;
	currentState_->EndState(*this);
	state_ = state;
	currentState_ = states_.at(state_).get();
	currentState_->StartState(*this);
}
