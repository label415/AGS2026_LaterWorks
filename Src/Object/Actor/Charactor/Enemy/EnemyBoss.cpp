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
#include "State/EnemyStateManager.h"
#include "State/BossIdleState.h"
#include "State/BossAttackState.h"
#include "State/BossChaseState.h"
#include "State/BossMoveState.h"
#include "EnemyBoss.h"

namespace
{
	constexpr const wchar_t* MODEL_ENEMY_BOSS_NAME = L"ModelEnemyBoss";
	constexpr const wchar_t* JSON_ENEMY_DATA_NAME = L"EnemyData";
	constexpr const wchar_t* ANIM_BOSS_IDLE = L"AnimationBossIdle";
	constexpr const wchar_t* ANIM_BOSS_MOVE = L"AnimationBossMove";
	constexpr const wchar_t* ANIM_BOSS_LEFT_ROT = L"AnimationBossLeftRot";
	constexpr const wchar_t* ANIM_BOSS_RIGHT_ROT = L"AnimationBossRightRot";
	constexpr const wchar_t* ANIM_BOSS_ATTACK = L"AnimationBossAttack";
	constexpr const wchar_t* ANIM_BOSS_COMBO = L"AnimationBossCombo";
	constexpr const wchar_t* ANIM_BOSS_DAMAGE = L"AnimationBossDamage";
}

EnemyBoss::EnemyBoss(void)
	: EnemyBase()
	, states_(nullptr)
	, blackBoard_()
	, targetState_(nullptr)
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
	states_->Release();
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
	const auto& anim = GetData()["animation"];

	auto addAnim = [&](ANIM_TYPE type, const char* key, const wchar_t* resName)
	{
		const auto& entry = anim[key];
		animationController_->Add(
			static_cast<int>(type),
			entry["speed"],
			resMng_.LoadModelDuplicate(resName),
			entry["blendTime"]);
	};

	addAnim(ANIM_TYPE::IDLE,      "idle",     ANIM_BOSS_IDLE);
	addAnim(ANIM_TYPE::WALK,      "walk",     ANIM_BOSS_MOVE);
	addAnim(ANIM_TYPE::RUN,       "run",      ANIM_BOSS_MOVE);
	addAnim(ANIM_TYPE::LEFT_ROT,  "leftRot",  ANIM_BOSS_LEFT_ROT);
	addAnim(ANIM_TYPE::RIGHT_ROT, "rightRot", ANIM_BOSS_RIGHT_ROT);
	addAnim(ANIM_TYPE::ATTACK,    "attack",   ANIM_BOSS_ATTACK);
	addAnim(ANIM_TYPE::COMBO,     "combo",    ANIM_BOSS_COMBO);
	addAnim(ANIM_TYPE::DAMAGE,    "damage",   ANIM_BOSS_DAMAGE);

	animationController_->Play(static_cast<int>(ANIM_TYPE::WALK));
}

void EnemyBoss::InitPost(void)
{
	weapons_->LoadEnd();

	SetBlackboard();

	InitState();

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

	// アニメーション速度・ブレンド時間反映
	if (animationController_)
	{
		const auto& anim = data["animation"];
		auto applyAnim = [&](ANIM_TYPE type, const char* key)
		{
			const auto& entry = anim[key];
			animationController_->SetSpeed(static_cast<int>(type), entry["speed"]);
			animationController_->SetBlendTime(static_cast<int>(type), entry["blendTime"]);
		};

		applyAnim(ANIM_TYPE::IDLE,      "idle");
		applyAnim(ANIM_TYPE::WALK,      "walk");
		applyAnim(ANIM_TYPE::RUN,       "run");
		applyAnim(ANIM_TYPE::LEFT_ROT,  "leftRot");
		applyAnim(ANIM_TYPE::RIGHT_ROT, "rightRot");
		applyAnim(ANIM_TYPE::ATTACK,    "attack");
		applyAnim(ANIM_TYPE::COMBO,     "combo");
		applyAnim(ANIM_TYPE::DAMAGE,    "damage");
	}
}

void EnemyBoss::UpdateGui(void)
{
#ifdef _DEBUG
	if (ImGui::CollapsingHeader("Runtime State", ImGuiTreeNodeFlags_DefaultOpen))
	{
		if (ImGui::DragFloat3("Current Pos", &transform_.pos.x, 1.0f)) transform_.Update();
		if (animationController_)
		{
			ImGui::Text("Anim Type: %d  Blending: %s (%.2f)",
				animationController_->GetPlayType(),
				animationController_->IsBlending() ? "ON" : "OFF",
				animationController_->GetBlendRate());
		}
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

	// 意思決定(次の状態へ遷移)
	int nextState = bossAi_.DecideNextState(blackBoard_);
	if (nextState != BossAl::INVALID_STATE)
	{
		states_->ChangerState(nextState);
		blackBoard_.currentState = states_->GetState();
		blackBoard_.stateElapsedTime = 0.0f;
		blackBoard_.isActionFinished = false;
	}

	// 状態更新
	states_->Update();

	// 移動量減少
	const auto& data = GetData();
	movePow_ = AsoUtility::Lerp(
		movePow_, AsoUtility::VECTOR_ZERO
		, data["movement"]["movePowDecrease"].get<float>());
}

void EnemyBoss::UpdateProcessPost(void)
{
	if (states_ != nullptr)
	{
		states_->UpdatePost();
	}
}

void EnemyBoss::UpdateHitCollider(void)
{
}

void EnemyBoss::InitState(void)
{
	const auto& data = GetData();
	// インスタンス生成
	states_ = std::make_unique<EnemyStateManager>(*this);

	// 待機
	states_->AddState(
		static_cast<int>(EnemyBoss::STATE::IDLE), std::make_unique<BossIdleState>());
	// 歩き
	states_->AddState(
		static_cast<int>(EnemyBoss::STATE::WALK)
		, std::make_unique<BossMoveState>(
			data["movement"]["speedWalk"].get<float>()
			, static_cast<int>(EnemyBoss::ANIM_TYPE::WALK)));
	// 走り
	states_->AddState(
		static_cast<int>(EnemyBoss::STATE::RUN)
		, std::make_unique<BossMoveState>(
			data["movement"]["speedRun"].get<float>()
			, static_cast<int>(EnemyBoss::ANIM_TYPE::RUN)));
	// 旋回
	states_->AddState(
		static_cast<int>(EnemyBoss::STATE::CHSDR), std::make_unique<BossChaseState>());
	// 攻撃
	states_->AddState(
		static_cast<int>(EnemyBoss::STATE::ATTACK)
		, std::make_unique<BossAttackState>(
			static_cast<int>(EnemyBoss::ANIM_TYPE::ATTACK)));

	states_->ChangerState(static_cast<int>(STATE::IDLE));
	blackBoard_.currentState = states_->GetState();

	// 意思決定AIの条件ルールを初期化
	bossAi_.Init();
}

void EnemyBoss::SetBlackboard(void)
{
	float dt = scnMng_.GetDeltaTime();

	// プレイヤーとの距離・正面からの角度を更新
	if (targetPos_ != nullptr)
	{
		blackBoard_.distance = static_cast<float>(
			AsoUtility::Distance(*targetPos_, transform_.pos));
		blackBoard_.angleToPlayer = static_cast<float>(
			AsoUtility::GetAconf(*targetPos_, transform_.pos, transform_.GetForward()));
	}

	// 残りHP割合を更新
	float maxHp = 100.0f;
	const auto& data = GetData();
	if (data.contains("status") && data["status"].contains("maxHp"))
	{
		maxHp = data["status"]["maxHp"].get<float>();
	}
	blackBoard_.hpRate = (maxHp > 0.0f) 
		? AsoUtility::Clamp(hp_ / maxHp, 0.0f, 1.0f) : 0.0f;

	// 現在の状態ID・経過時間・行動完了フラグを更新
	if (states_ != nullptr)
	{
		int currentState = states_->GetState();
		if (blackBoard_.currentState != currentState)
		{
			blackBoard_.currentState = currentState;
			blackBoard_.stateElapsedTime = 0.0f;
			blackBoard_.isActionFinished = false;
		}
		else
		{
			blackBoard_.stateElapsedTime += dt;
		}
	}

	// 相手の現在状態・過去履歴を更新
	if (targetState_ != nullptr)
	{
		int playerState = *targetState_;
		if (blackBoard_.currentStatePlayer != playerState)
		{
			blackBoard_.currentStatesPlayer.push_back(blackBoard_.currentStatePlayer);
			if (blackBoard_.currentStatesPlayer.size() > MAX_PLAYER_STATE_HISTORY)
			{
				blackBoard_.currentStatesPlayer.erase(blackBoard_.currentStatesPlayer.begin());
			}
			blackBoard_.currentStatePlayer = playerState;
		}
	}

	// クールタイムを経過時間分だけ減算
	blackBoard_.UpdateCoolTimes(dt);
}

void EnemyBoss::SetTargetDir(void)
{
	// 追尾対象の方向を設定
	VECTOR dir = VSub(*targetPos_, transform_.pos);
	dir.y = 0.0f;
	moveDir_ = AsoUtility::VNormalize(dir);
}
