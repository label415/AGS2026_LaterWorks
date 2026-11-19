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
#include "AI/AiCondition.h"
#include "AI/AiUtility.h"
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

	const int walkType = static_cast<int>(ANIM_TYPE::WALK);
	animationController_->Play(walkType, true, false, animationController_->GetBlendTime(walkType));
}

void EnemyBoss::InitPost(void)
{
	weapons_->LoadEnd();

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

	if (ImGui::CollapsingHeader("AI State (HFSM / Utility)", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::Text("Hierarchy : %s", hfsm_.GetCurrentHierarchyPath().c_str());
		ImGui::Text("Last Trans: %s", hfsm_.GetLastTransitionLabel().c_str());
		ImGui::Text("Distance  : %.1f  Angle: %.1f deg  Elapsed: %.2f s",
			blackboard_.distance, blackboard_.angleToPlayer, blackboard_.stateElapsedTime);

		const auto& scores = hfsm_.GetLastUtilityScores();
		if (!scores.empty())
		{
			ImGui::Separator();
			ImGui::Text("Utility Scores:");
			for (const auto& s : scores)
			{
				ImGui::Text("  %-12s : %.3f", s.name.c_str(), s.score);
			}
		}
	}
#endif
}

void EnemyBoss::UpdateProcess(void)
{
	SetBlackboard();

	// 遷移ルール(HFSM条件 + Utility評価)を一括判定
	if (hfsm_.EvaluateTransitions(*this, blackboard_))
	{
		blackboard_.currentState = hfsm_.GetCurrentStateId();
		blackboard_.stateElapsedTime = 0.0f;
		blackboard_.isActionFinished = false;
	}

	// 現在ステートの行動処理を実行
	hfsm_.Update(*this);

	const auto& data = GetData();
	movePow_ = AsoUtility::Lerp(
		movePow_, AsoUtility::VECTOR_ZERO, data["movement"]["movePowDecrease"].get<float>());
}

void EnemyBoss::UpdateProcessPost(void)
{
	hfsm_.UpdatePost(*this);
}

void EnemyBoss::UpdateHitCollider(void)
{
}

void EnemyBoss::InitState(void)
{
	hfsm_.Clear();

	constexpr int GROUP_ALIVE   = static_cast<int>(STATE_GROUP::ALIVE);
	constexpr int GROUP_NEUTRAL = static_cast<int>(STATE_GROUP::NEUTRAL);
	constexpr int GROUP_COMBAT  = static_cast<int>(STATE_GROUP::COMBAT);

	// HFSMの階層グループ登録
	hfsm_.RegisterGroup(GROUP_ALIVE,   "ALIVE");
	hfsm_.RegisterGroup(GROUP_NEUTRAL, "NEUTRAL", GROUP_ALIVE);
	hfsm_.RegisterGroup(GROUP_COMBAT,  "COMBAT",  GROUP_ALIVE);

	// 各実体ステートを所属グループに登録
	hfsm_.RegisterState(static_cast<int>(STATE::IDLE),   std::make_unique<BossIdleState>(),   "IDLE",   GROUP_NEUTRAL);
	hfsm_.RegisterState(static_cast<int>(STATE::MOVE),   std::make_unique<BossMoveState>(),   "MOVE",   GROUP_NEUTRAL);
	hfsm_.RegisterState(static_cast<int>(STATE::CHSDR),  std::make_unique<BossChaseState>(),  "CHASE",  GROUP_NEUTRAL);
	hfsm_.RegisterState(static_cast<int>(STATE::ATTACK), std::make_unique<BossAttackState>(), "ATTACK", GROUP_COMBAT);

	// 遷移条件(Condition)とUtility選択ルールを一括登録
	InitTransitions();

	// 初期状態を開始
	hfsm_.Start(*this, static_cast<int>(STATE::IDLE));
	blackboard_.currentState = hfsm_.GetCurrentStateId();
	blackboard_.stateElapsedTime = 0.0f;
	blackboard_.isActionFinished = false;
}

void EnemyBoss::InitTransitions(void)
{
	using namespace EnemyConditions;
	using namespace EnemyConsiderations;

	constexpr int GROUP_ALIVE   = static_cast<int>(STATE_GROUP::ALIVE);
	constexpr int GROUP_COMBAT  = static_cast<int>(STATE_GROUP::COMBAT);
	constexpr int ST_IDLE       = static_cast<int>(STATE::IDLE);
	constexpr int ST_MOVE       = static_cast<int>(STATE::MOVE);
	constexpr int ST_CHASE      = static_cast<int>(STATE::CHSDR);
	constexpr int ST_DIE        = static_cast<int>(STATE::DIE);

	// 再利用可能な共通条件パーツ
	auto isInFov   = InFov(FOV_HALF_ANGLE);
	auto isOutFov  = !isInFov;
	auto isFar     = DistanceGreater(500.0f);
	auto isArrived = DistanceLessEq(400.0f);

	// --- 1. 親グループ単位の共通遷移(状態が増えても1行で全配下に適用) ---
	// 生存中グループ全体から死亡への割り込み(DIEステート追加後に自動有効化)
	hfsm_.AddTransition(GROUP_ALIVE, ST_DIE, 100, IsDead(), "ALIVE -> DIE");
	// 攻撃グループ全体から行動完了時に待機へ復帰
	hfsm_.AddTransition(GROUP_COMBAT, ST_IDLE, 60, IsActionFinished(), "COMBAT -> IDLE");

	// --- 2. 個別ステートの終了・復帰遷移 ---
	hfsm_.AddTransition(ST_CHASE, ST_IDLE, 50, isInFov,   "CHASE -> IDLE");
	hfsm_.AddTransition(ST_MOVE,  ST_IDLE, 50, isArrived, "MOVE -> IDLE");

	// --- 3. IDLE状態からの行動決定(Utility AIでスコア評価して最良行動を選択) ---
	auto idleSelector = std::make_shared<EnemyUtilitySelector>();

	// 視野角外なら最優先で旋回(CHASE)
	idleSelector->AddOption(ST_CHASE, 1.0f, "TurnToTarget", {
		FromCondition(isOutFov)
	});

	// 正面かつ一定距離以上離れていれば距離に応じたスコアで接近(MOVE)
	idleSelector->AddOption(ST_MOVE, 0.8f, "Approach", {
		FromCondition(isInFov && isFar),
		DistanceLinear(400.0f, 1200.0f, false)
	});

	// 今後 ATTACK / COMBO 等を増やす場合は idleSelector->AddOption(...) を追加するだけで拡張可能

	hfsm_.AddUtilityTransition(ST_IDLE, 20, idleSelector, 0.0f, false, "IDLE Utility");
}

void EnemyBoss::SetBlackboard(void)
{
	float dt = scnMng_.GetDeltaTime();
	blackboard_.stateElapsedTime += dt;
	blackboard_.UpdateCoolTimes(dt);
	blackboard_.currentState = hfsm_.GetCurrentStateId();

	if (targetPos_ != nullptr)
	{
		blackboard_.angleToPlayer = static_cast<float>(
			AsoUtility::GetAconf(*targetPos_, transform_.pos, transform_.GetForward()));
		blackboard_.distance = static_cast<float>(
			AsoUtility::Distance(*targetPos_, transform_.pos));
	}
}

void EnemyBoss::SetTargetDir(void)
{
	VECTOR dir = VSub(*targetPos_, transform_.pos);
	dir.y = 0.0f;
	moveDir_ = AsoUtility::VNormalize(dir);
}

void EnemyBoss::ChangeState(int state)
{
	if (hfsm_.ChangeState(*this, state))
	{
		blackboard_.currentState = hfsm_.GetCurrentStateId();
		blackboard_.stateElapsedTime = 0.0f;
		blackboard_.isActionFinished = false;
	}
}
