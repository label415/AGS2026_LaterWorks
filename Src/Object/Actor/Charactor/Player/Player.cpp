#include <fstream>
#include <algorithm>
#include <DxLib.h>
#include "../../../../Libs/ImGui/imgui.h"
#include "../../../../Manager/ResourceManager.h"
#include "../../../../Manager/SceneManager.h"
#include "../../../../Manager/CollisionManager.h"
#include "../../../../Manager/Camera.h"
#include "../../../../Manager/InputManager.h"
#include "../../../../Manager/SoundManager.h"
#include "../../../../Manager/UiManager.h"
#include "../../../../Manager/EditorManager.h"
#include "../../../../Utility/AsoUtility.h"
#include "../../../../Utility/ColliderUtility.h"
#include "../../../../Utility/MatrixUtility.h"
#include "../../../../Application.h"
#include "../../../../Common/Quaternion.h"
#include "../../../Common/Collider/ColliderLine.h"
#include "../../../Common/Collider/ColliderCapsule.h"
#include "../../../Common/Collider/ColliderSphere.h"
#include "../../../Common/Collider/ColliderModel.h"
#include "../../Weapon/WeaponSaw.h"
#include "../../UI/UiSt.h"
#include "../../UI/UiHp.h"
#include "../../UI/UiRecovery.h"
#include "State/PlayerStateBase.h"
#include "State/PlayerMoveState.h"
#include "State/PlayerIdleState.h"
#include "State/PlayerAttackState.h"
#include "State/PlayerEvasionState.h"
#include "State/PlayerDamageState.h"
#include "State/PlayerRecoveryState.h"
#include "Player.h"

// リソース名
namespace
{
	// プレイヤーモデル
	constexpr const wchar_t* MODEL_PLAYER_NAME = L"ModelPlayer";
	// プレイヤーアニメーション(待機)
	constexpr const wchar_t* ANIM_PLAYER_IDLE_NAME = L"AnimationPlayerIdle";
	// プレイヤーアニメーション(歩き)
	constexpr const wchar_t* ANIM_PLAYER_WALK_NAME = L"AnimationPlayerWalk";
	// プレイヤーアニメーション(走り)
	constexpr const wchar_t* ANIM_PLAYER_RUN_NAME = L"AnimationPlayerRun";
	// プレイヤーアニメーション(攻撃1)
	constexpr const wchar_t* ANIM_PLAYER_ATTACK_1_NAME = L"AnimationPlayerAttack1";
	// プレイヤーアニメーション(攻撃2)
	constexpr const wchar_t* ANIM_PLAYER_ATTACK_2_NAME = L"AnimationPlayerAttack2";
	// プレイヤーアニメーション(攻撃3)
	constexpr const wchar_t* ANIM_PLAYER_ATTACK_3_NAME = L"AnimationPlayerAttack3";
	// プレイヤーアニメーション(回避)
	constexpr const wchar_t* ANIM_PLAYER_EVASION_NAME = L"AnimationPlayerEvasion";
	// プレイヤーアニメーション(ダウン)
	constexpr const wchar_t* ANIM_PLAYER_DOWN_NAME = L"AnimationPlayerDown";
	// プレイヤーアニメーション(起き上がり)
	constexpr const wchar_t* ANIM_PLAYER_UP_NAME = L"AnimationPlayerUp";
	// プレイヤーアニメーション(回復)
	constexpr const wchar_t* ANIM_PLAYER_RECOVERY_NAME = L"AnimationPlayerRecovery";
	// プレイヤーデータ
	constexpr const wchar_t* JSON_PLAYER_DATA_NAME = L"PlayerData";

	// サウンド
	// 歩きSE
	constexpr const wchar_t* SE_PLAYER_WAKE_NAME = L"SePlayerWake";
	// 走りSE
	constexpr const wchar_t* SE_PLAYER_RUN_NAME = L"SePlayerRun";
	// 回復SE
	constexpr const wchar_t* SE_PLAYER_RECOVERY_NAME = L"SePlayerRecovery";
	// 回避SE
	constexpr const wchar_t* SE_PLAYER_EVASION_NAME = L"SePlayerEvasion";
	// 攻撃SE
	constexpr const wchar_t* SE_PLAYER_WEAPON_1_NAME = L"SePlayerWeapon1";
	// 被ダメージSE
	constexpr const wchar_t* SE_PLAYER_DAMAGE_NAME = L"SePlayerDamage";
}

Player::Player(void)
	: CharactorBase()
	, param_()
	, animLockPos_(AsoUtility::VECTOR_ZERO)
	, effType_(EFFECT::NONE)
	, invincibleTimer_(0.0f)
	, isComboNext_(false)
	, stateAtkCombo_(STATE_ATTACK_COMBO::COMBO_1)
	, state_(STATE::IDLE)
	, stamina_(100.0f)
	, stCoolTime_(0.0f)
	, currentState_(nullptr)
	, recoveryBottleCount_(6)
{
}

Player::~Player(void) {}

void Player::Draw(void)
{
	// 基底クラスの描画処理
	CharactorBase::Draw();

	// 武器描画
	for (const auto& weapon : weapons_) {
		weapon->Draw();
	}
}

void Player::Release(void)
{
#ifdef _DEBUG
	EditorManager::GetInstance().UnregisterObject("Player");
#endif
	// 基底クラスの解放処理
	CharactorBase::Release();
}

void Player::InitLoad(void)
{
	// JSONからパラメータを読み込む
	const auto& playerJson = resMng_.Load(JSON_PLAYER_DATA_NAME).jsonData_;
	LoadParam(playerJson);

	// HP・スタミナ・回復瓶の初期値を設定
	hp_ = param_.maxHp;
	stamina_ = param_.maxStamina;
	recoveryBottleCount_ = param_.initialRecoveryBottleCount;

	// プレイヤーモデル
	transform_.SetModel(resMng_.LoadModelDuplicate(
		MODEL_PLAYER_NAME));

	// 武器初期化
	InitWeapon();

	// サウンド初期化
	InitSound();

	// プレイヤーUI
	auto& ui = UiManager::GetInstance();

	// プレイヤーHPバー
	auto platerHp = std::make_unique<UiHp>(
		&hp_, Application::ASPECT_RATIO_X / 4, 20.0f, 2.9f, 4.0f, 0.7f);
	ui.Add(std::move(platerHp));

	// プレイヤーSTバー
	auto platerSt = std::make_unique<UiSt>(
		&stamina_, Application::ASPECT_RATIO_X / 4, 60.0f, 2.9f, 4.0f, 0.7f);
	ui.Add(std::move(platerSt));
}

void Player::InitTransform(void)
{
	// 座標・大きさ・回転の初期化
	transform_.pos = param_.initPos;
	transform_.scl = param_.initScale;
	transform_.quaRot = Quaternion::Identity();
	transform_.quaRotLocal = Quaternion::AngleAxis(
		AsoUtility::Deg2RadF(param_.initAngleY), AsoUtility::AXIS_Y);

	// Transformの更新
	transform_.Update();
}

void Player::InitCollider(void)
{
	// 線分コライダ
	std::vector<ColliderBase::TAG> lineTags = { ColliderBase::TAG::STAGE };
	auto colLine = std::make_shared<ColliderLine>(
		ColliderBase::TAG::GROUND,
		lineTags,
		&transform_,
		param_.colLineStart,
		param_.colLineEnd);

	// マネージャーへ登録
	ownColliders_[static_cast<int>(ColliderBase::SHAPE::LINE)].push_back(colLine);
	colMng_.AddCollider(colLine);

	// カプセルコライダ
	std::vector<ColliderBase::TAG> capsuleTags = {
		ColliderBase::TAG::ENEMY,
		ColliderBase::TAG::ENEMY_WEPON,
		ColliderBase::TAG::STAGE
	};

	auto colCapsule = std::make_shared<ColliderCapsule>(
		ColliderBase::TAG::PLAYER,
		capsuleTags,
		&transform_,
		param_.colCapsuleTop,
		param_.colCapsuleDown,
		param_.colCapsuleRadius);

	// マネージャーへ登録
	ownColliders_[static_cast<int>(ColliderBase::SHAPE::CAPSULE)].push_back(colCapsule);
	colMng_.AddCollider(colCapsule);
}

void Player::InitAnimation(void)
{
	// アニメーション
	animationController_ = std::make_unique<AnimationController>(transform_.modelId);

	// アニメーションの登録
	animationController_->Add(static_cast<int>(ANIM_TYPE::IDLE),
		param_.animSpeedIdle, resMng_.LoadModelDuplicate(ANIM_PLAYER_IDLE_NAME));
	animationController_->Add(static_cast<int>(ANIM_TYPE::RUN),
		param_.animSpeedRun, resMng_.LoadModelDuplicate(ANIM_PLAYER_RUN_NAME));
	animationController_->Add(static_cast<int>(ANIM_TYPE::FAST_RUN),
		param_.animSpeedFastRun, resMng_.LoadModelDuplicate(ANIM_PLAYER_RUN_NAME));
	animationController_->Add(static_cast<int>(ANIM_TYPE::ATTACK_1),
		param_.animSpeedAttack1, resMng_.LoadModelDuplicate(ANIM_PLAYER_ATTACK_1_NAME));
	animationController_->Add(static_cast<int>(ANIM_TYPE::ATTACK_2),
		param_.animSpeedAttack2, resMng_.LoadModelDuplicate(ANIM_PLAYER_ATTACK_2_NAME));
	animationController_->Add(static_cast<int>(ANIM_TYPE::ATTACK_3),
		param_.animSpeedAttack3, resMng_.LoadModelDuplicate(ANIM_PLAYER_ATTACK_3_NAME));
	animationController_->Add(static_cast<int>(ANIM_TYPE::EVASION),
		param_.animSpeedEvasion, resMng_.LoadModelDuplicate(ANIM_PLAYER_EVASION_NAME));
	animationController_->Add(static_cast<int>(ANIM_TYPE::DOWN),
		param_.animSpeedDown, resMng_.LoadModelDuplicate(ANIM_PLAYER_DOWN_NAME));
	animationController_->Add(static_cast<int>(ANIM_TYPE::UP),
		param_.animSpeedUp, resMng_.LoadModelDuplicate(ANIM_PLAYER_UP_NAME));
	animationController_->Add(static_cast<int>(ANIM_TYPE::RECOVERY),
		param_.animSpeedRecovery, resMng_.LoadModelDuplicate(ANIM_PLAYER_RECOVERY_NAME));

	// 初期アニメーション再生
	animationController_->Play(static_cast<int>(ANIM_TYPE::IDLE));
}

void Player::InitPost(void)
{
	// 武器
	for (const auto& weapon : weapons_) {
		weapon->LoadEnd();
	}

	// エフェクト
	effType_ = EFFECT::NONE;
	effect_ = std::make_unique<EffectController>();
	effect_->Add(
		static_cast<int>(EFFECT::BLOOD),
		(Application::PATH_EFFECT + L"Blood.efkefc"));
	effect_->Add(
		static_cast<int>(EFFECT::HP_ABSOLUTE),
		(Application::PATH_EFFECT + L"Absolute.efkefc"));

	// 攻撃コンボ状態の初期化
	stateAtkCombo_ = STATE_ATTACK_COMBO::COMBO_1;

	// 攻撃コンボの次の攻撃を受け付けるかどうかのフラグを初期化
	isComboNext_ = false;

	// 各状態格納
	InitState();

#ifdef _DEBUG
	// 統合エディタへPlayerを登録
	EditorManager::EditObject editObj;
	editObj.name = "Player";
	editObj.jsonSrc = JSON_PLAYER_DATA_NAME;
	editObj.drawGui = [this]() { this->UpdateGui(); };
	editObj.saveToJson = [this](Json& json) { this->SaveParam(json); };
	editObj.loadFromJson = [this](const Json& json) {
		this->LoadParam(json);
		this->ApplyParam(true);
	};
	EditorManager::GetInstance().RegisterObject(editObj);
#endif
}

void Player::LoadParam(const Json& json)
{
	// Transform
	if (json.contains("transform"))
	{
		const auto& t = json["transform"];
		if (t.contains("position"))
		{
			param_.initPos = { t["position"]["x"], t["position"]["y"], t["position"]["z"] };
		}
		if (t.contains("scale"))
		{
			param_.initScale = { t["scale"]["x"], t["scale"]["y"], t["scale"]["z"] };
		}
		if (t.contains("angleY"))
		{
			param_.initAngleY = t["angleY"];
		}
	}

	// Collider
	if (json.contains("collider"))
	{
		const auto& c = json["collider"];
		if (c.contains("line"))
		{
			const auto& l = c["line"];
			param_.colLineStart = { l["startPos"]["x"], l["startPos"]["y"], l["startPos"]["z"] };
			param_.colLineEnd = { l["endPos"]["x"], l["endPos"]["y"], l["endPos"]["z"] };
		}
		if (c.contains("capsule"))
		{
			const auto& cap = c["capsule"];
			param_.colCapsuleTop = { cap["topPos"]["x"], cap["topPos"]["y"], cap["topPos"]["z"] };
			param_.colCapsuleDown = { cap["downPos"]["x"], cap["downPos"]["y"], cap["downPos"]["z"] };
			param_.colCapsuleRadius = cap["radius"];
		}
	}

	// Status
	if (json.contains("status"))
	{
		const auto& s = json["status"];
		param_.maxHp = s.value("maxHp", param_.maxHp);
		param_.maxStamina = s.value("maxStamina", param_.maxStamina);
		param_.recoveryStSpeed = s.value("recoveryStSpeed", param_.recoveryStSpeed);
		param_.consumptionStFastRun = s.value("consumptionStFastRun", param_.consumptionStFastRun);
		param_.consumptionStEvasion = s.value("consumptionStEvasion", param_.consumptionStEvasion);
		param_.consumptionStAttack = s.value("consumptionStAttack", param_.consumptionStAttack);
		param_.staminaExhaustionTime = s.value("staminaExhaustionTime", param_.staminaExhaustionTime);
		param_.invincibleTime = s.value("invincibleTime", param_.invincibleTime);
		param_.healHpAmount = s.value("healHpAmount", param_.healHpAmount);
		param_.initialRecoveryBottleCount = s.value("initialRecoveryBottleCount", param_.initialRecoveryBottleCount);
		param_.consumeRecoveryBottleCount = s.value("consumeRecoveryBottleCount", param_.consumeRecoveryBottleCount);
	}

	// Movement
	if (json.contains("movement"))
	{
		const auto& m = json["movement"];
		param_.speedMove = m.value("speedMove", param_.speedMove);
		param_.speedDash = m.value("speedDash", param_.speedDash);
		param_.speedEvasion = m.value("speedEvasion", param_.speedEvasion);
		param_.movePowDecrease = m.value("movePowDecrease", param_.movePowDecrease);
		param_.avoidanceAngleAxisY = m.value("avoidanceAngleAxisY", param_.avoidanceAngleAxisY);
		param_.rotTargetSlerpRate = m.value("rotTargetSlerpRate", param_.rotTargetSlerpRate);
		param_.attackStepLerpRate = m.value("attackStepLerpRate", param_.attackStepLerpRate);
	}

	// Combat
	if (json.contains("combat"))
	{
		const auto& cb = json["combat"];
		param_.damageEnemyBody = cb.value("damageEnemyBody", param_.damageEnemyBody);
		param_.damageEnemyWeapon = cb.value("damageEnemyWeapon", param_.damageEnemyWeapon);
		param_.pushBackPower = cb.value("pushBackPower", param_.pushBackPower);
	}

	// Effect & Sound
	if (json.contains("effectAndSound"))
	{
		const auto& es = json["effectAndSound"];
		param_.seVolume = es.value("seVolume", param_.seVolume);
		if (es.contains("effectBloodScale"))
		{
			param_.effectBloodScale = {
				es["effectBloodScale"]["x"],
				es["effectBloodScale"]["y"],
				es["effectBloodScale"]["z"]
			};
		}
		param_.effectRecoveryScale = es.value("effectRecoveryScale", param_.effectRecoveryScale);
		param_.effectRecoveryOffsetY = es.value("effectRecoveryOffsetY", param_.effectRecoveryOffsetY);
		param_.dieFadeSpeed = es.value("dieFadeSpeed", param_.dieFadeSpeed);
		param_.dieEndThreshold = es.value("dieEndThreshold", param_.dieEndThreshold);
		param_.attackSeTriggerStep = es.value("attackSeTriggerStep", param_.attackSeTriggerStep);
		param_.recoveryTriggerStep = es.value("recoveryTriggerStep", param_.recoveryTriggerStep);
		param_.weaponAttachFrameNo = es.value("weaponAttachFrameNo", param_.weaponAttachFrameNo);
		param_.hitEffectFrameNo = es.value("hitEffectFrameNo", param_.hitEffectFrameNo);
		param_.upLockPosYAdd = es.value("upLockPosYAdd", param_.upLockPosYAdd);
		if (es.contains("lockPos1"))
		{
			param_.lockPos1 = {
				es["lockPos1"]["x"],
				es["lockPos1"]["y"],
				es["lockPos1"]["z"]
			};
		}
		param_.lockFrameNo = es.value("lockFrameNo", param_.lockFrameNo);
	}

	// Animation Speed
	if (json.contains("animationSpeed"))
	{
		const auto& anim = json["animationSpeed"];
		param_.animSpeedIdle = anim.value("idle", param_.animSpeedIdle);
		param_.animSpeedRun = anim.value("run", param_.animSpeedRun);
		param_.animSpeedFastRun = anim.value("fastRun", param_.animSpeedFastRun);
		param_.animSpeedAttack1 = anim.value("attack1", param_.animSpeedAttack1);
		param_.animSpeedAttack2 = anim.value("attack2", param_.animSpeedAttack2);
		param_.animSpeedAttack3 = anim.value("attack3", param_.animSpeedAttack3);
		param_.animSpeedEvasion = anim.value("evasion", param_.animSpeedEvasion);
		param_.animSpeedDown = anim.value("down", param_.animSpeedDown);
		param_.animSpeedUp = anim.value("up", param_.animSpeedUp);
		param_.animSpeedRecovery = anim.value("recovery", param_.animSpeedRecovery);
	}

	// Combo
	if (json.contains("combo"))
	{
		atkComboData_.clear();
		const auto& playerCombo = json["combo"];
		const char* keys[] = { "combo1", "combo2", "combo3" };
		for (int i = 0; i < 3; ++i)
		{
			if (!playerCombo.contains(keys[i])) continue;
			const auto& combo = playerCombo[keys[i]];
			ATTACK_COMBO data = {
				static_cast<ANIM_TYPE>(combo.value("animType", 4 + i)),
				combo.value("inputStart", 0.0f),
				combo.value("inputEnd", 0.0f),
				combo.value("colStart", 0.0f),
				combo.value("colEnd", 0.0f),
				combo.value("stepInterrupt", 0.0f),
				combo.value("moveSpeed", 0.0f),
				static_cast<STATE_ATTACK_COMBO>(combo.value("nextCombo", 2))
			};
			atkComboData_.emplace(static_cast<STATE_ATTACK_COMBO>(i), data);
		}
	}
}

void Player::SaveParam(Json& json) const
{
	// Transform
	json["transform"]["position"] = {
		{ "x", param_.initPos.x },
		{ "y", param_.initPos.y },
		{ "z", param_.initPos.z }
	};
	json["transform"]["scale"] = {
		{ "x", param_.initScale.x },
		{ "y", param_.initScale.y },
		{ "z", param_.initScale.z }
	};
	json["transform"]["angleY"] = param_.initAngleY;

	// Collider
	json["collider"]["line"]["startPos"] = {
		{ "x", param_.colLineStart.x },
		{ "y", param_.colLineStart.y },
		{ "z", param_.colLineStart.z }
	};
	json["collider"]["line"]["endPos"] = {
		{ "x", param_.colLineEnd.x },
		{ "y", param_.colLineEnd.y },
		{ "z", param_.colLineEnd.z }
	};
	json["collider"]["capsule"]["topPos"] = {
		{ "x", param_.colCapsuleTop.x },
		{ "y", param_.colCapsuleTop.y },
		{ "z", param_.colCapsuleTop.z }
	};
	json["collider"]["capsule"]["downPos"] = {
		{ "x", param_.colCapsuleDown.x },
		{ "y", param_.colCapsuleDown.y },
		{ "z", param_.colCapsuleDown.z }
	};
	json["collider"]["capsule"]["radius"] = param_.colCapsuleRadius;

	// Status
	json["status"]["maxHp"] = param_.maxHp;
	json["status"]["maxStamina"] = param_.maxStamina;
	json["status"]["recoveryStSpeed"] = param_.recoveryStSpeed;
	json["status"]["consumptionStFastRun"] = param_.consumptionStFastRun;
	json["status"]["consumptionStEvasion"] = param_.consumptionStEvasion;
	json["status"]["consumptionStAttack"] = param_.consumptionStAttack;
	json["status"]["staminaExhaustionTime"] = param_.staminaExhaustionTime;
	json["status"]["invincibleTime"] = param_.invincibleTime;
	json["status"]["healHpAmount"] = param_.healHpAmount;
	json["status"]["initialRecoveryBottleCount"] = param_.initialRecoveryBottleCount;
	json["status"]["consumeRecoveryBottleCount"] = param_.consumeRecoveryBottleCount;

	// Movement
	json["movement"]["speedMove"] = param_.speedMove;
	json["movement"]["speedDash"] = param_.speedDash;
	json["movement"]["speedEvasion"] = param_.speedEvasion;
	json["movement"]["movePowDecrease"] = param_.movePowDecrease;
	json["movement"]["avoidanceAngleAxisY"] = param_.avoidanceAngleAxisY;
	json["movement"]["rotTargetSlerpRate"] = param_.rotTargetSlerpRate;
	json["movement"]["attackStepLerpRate"] = param_.attackStepLerpRate;

	// Combat
	json["combat"]["damageEnemyBody"] = param_.damageEnemyBody;
	json["combat"]["damageEnemyWeapon"] = param_.damageEnemyWeapon;
	json["combat"]["pushBackPower"] = param_.pushBackPower;

	// Effect & Sound
	json["effectAndSound"]["seVolume"] = param_.seVolume;
	json["effectAndSound"]["effectBloodScale"] = {
		{ "x", param_.effectBloodScale.x },
		{ "y", param_.effectBloodScale.y },
		{ "z", param_.effectBloodScale.z }
	};
	json["effectAndSound"]["effectRecoveryScale"] = param_.effectRecoveryScale;
	json["effectAndSound"]["effectRecoveryOffsetY"] = param_.effectRecoveryOffsetY;
	json["effectAndSound"]["dieFadeSpeed"] = param_.dieFadeSpeed;
	json["effectAndSound"]["dieEndThreshold"] = param_.dieEndThreshold;
	json["effectAndSound"]["attackSeTriggerStep"] = param_.attackSeTriggerStep;
	json["effectAndSound"]["recoveryTriggerStep"] = param_.recoveryTriggerStep;
	json["effectAndSound"]["weaponAttachFrameNo"] = param_.weaponAttachFrameNo;
	json["effectAndSound"]["hitEffectFrameNo"] = param_.hitEffectFrameNo;
	json["effectAndSound"]["upLockPosYAdd"] = param_.upLockPosYAdd;
	json["effectAndSound"]["lockPos1"] = {
		{ "x", param_.lockPos1.x },
		{ "y", param_.lockPos1.y },
		{ "z", param_.lockPos1.z }
	};
	json["effectAndSound"]["lockFrameNo"] = param_.lockFrameNo;

	// Animation Speed
	json["animationSpeed"]["idle"] = param_.animSpeedIdle;
	json["animationSpeed"]["run"] = param_.animSpeedRun;
	json["animationSpeed"]["fastRun"] = param_.animSpeedFastRun;
	json["animationSpeed"]["attack1"] = param_.animSpeedAttack1;
	json["animationSpeed"]["attack2"] = param_.animSpeedAttack2;
	json["animationSpeed"]["attack3"] = param_.animSpeedAttack3;
	json["animationSpeed"]["evasion"] = param_.animSpeedEvasion;
	json["animationSpeed"]["down"] = param_.animSpeedDown;
	json["animationSpeed"]["up"] = param_.animSpeedUp;
	json["animationSpeed"]["recovery"] = param_.animSpeedRecovery;

	// Combo
	const char* keys[] = { "combo1", "combo2", "combo3" };
	for (int i = 0; i < 3; ++i)
	{
		auto it = atkComboData_.find(static_cast<STATE_ATTACK_COMBO>(i));
		if (it == atkComboData_.end()) continue;
		const auto& c = it->second;
		json["combo"][keys[i]] = {
			{ "animType", static_cast<int>(c.animType) },
			{ "inputStart", c.stepInputStart },
			{ "inputEnd", c.stepInputEnd },
			{ "colStart", c.stepCollisionStart },
			{ "colEnd", c.stepCollisionEnd },
			{ "stepInterrupt", c.stepInterrupt },
			{ "moveSpeed", c.moveSpeed },
			{ "nextCombo", static_cast<int>(c.nextCombo) }
		};
	}
}

void Player::ApplyParam(bool resetTransformPos)
{
	// Transform反映
	if (resetTransformPos)
	{
		transform_.pos = param_.initPos;
	}
	transform_.scl = param_.initScale;
	transform_.quaRotLocal = Quaternion::AngleAxis(
		AsoUtility::Deg2RadF(param_.initAngleY), AsoUtility::AXIS_Y);
	transform_.Update();

	// 線分コライダー反映
	int lineType = static_cast<int>(ColliderBase::SHAPE::LINE);
	if (ownColliders_.count(lineType) > 0)
	{
		for (auto& col : ownColliders_.at(lineType))
		{
			auto line = std::dynamic_pointer_cast<ColliderLine>(col);
			if (line)
			{
				line->SetLocalPosStart(param_.colLineStart);
				line->SetLocalPosEnd(param_.colLineEnd);
			}
		}
	}

	// カプセルコライダー反映
	int capType = static_cast<int>(ColliderBase::SHAPE::CAPSULE);
	if (ownColliders_.count(capType) > 0)
	{
		for (auto& col : ownColliders_.at(capType))
		{
			auto cap = std::dynamic_pointer_cast<ColliderCapsule>(col);
			if (cap)
			{
				cap->SetLocalPosTop(param_.colCapsuleTop);
				cap->SetLocalPosDown(param_.colCapsuleDown);
				cap->SetRadius(param_.colCapsuleRadius);
			}
		}
	}

	// アニメーション速度反映
	if (animationController_)
	{
		animationController_->SetSpeed(static_cast<int>(ANIM_TYPE::IDLE), param_.animSpeedIdle);
		animationController_->SetSpeed(static_cast<int>(ANIM_TYPE::RUN), param_.animSpeedRun);
		animationController_->SetSpeed(static_cast<int>(ANIM_TYPE::FAST_RUN), param_.animSpeedFastRun);
		animationController_->SetSpeed(static_cast<int>(ANIM_TYPE::ATTACK_1), param_.animSpeedAttack1);
		animationController_->SetSpeed(static_cast<int>(ANIM_TYPE::ATTACK_2), param_.animSpeedAttack2);
		animationController_->SetSpeed(static_cast<int>(ANIM_TYPE::ATTACK_3), param_.animSpeedAttack3);
		animationController_->SetSpeed(static_cast<int>(ANIM_TYPE::EVASION), param_.animSpeedEvasion);
		animationController_->SetSpeed(static_cast<int>(ANIM_TYPE::DOWN), param_.animSpeedDown);
		animationController_->SetSpeed(static_cast<int>(ANIM_TYPE::UP), param_.animSpeedUp);
		animationController_->SetSpeed(static_cast<int>(ANIM_TYPE::RECOVERY), param_.animSpeedRecovery);
	}

	// HP・スタミナ上限クランプ
	if (hp_ > param_.maxHp) hp_ = param_.maxHp;
	if (stamina_ > param_.maxStamina) stamina_ = param_.maxStamina;
}

void Player::UpdateGui(void)
{
#ifdef _DEBUG
	bool changed = false;

	// 1. 現在のランタイム状態
	if (ImGui::CollapsingHeader("Runtime State", ImGuiTreeNodeFlags_DefaultOpen))
	{
		if (ImGui::DragFloat3("Current Position", &transform_.pos.x, 1.0f))
		{
			transform_.Update();
		}
		ImGui::SliderFloat("Current HP", &hp_, 0.0f, param_.maxHp);
		ImGui::SliderFloat("Current Stamina", &stamina_, 0.0f, param_.maxStamina);
		ImGui::InputInt("Recovery Bottles", &recoveryBottleCount_);
		if (ImGui::Button("Full Heal HP & Stamina"))
		{
			hp_ = param_.maxHp;
			stamina_ = param_.maxStamina;
			recoveryBottleCount_ = param_.initialRecoveryBottleCount;
		}
	}

	// 2. Transform (初期配置・スケール・角度)
	if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::DragFloat3("Init Position", &param_.initPos.x, 5.0f);
		if (ImGui::Button("Set Current Pos -> Init Pos"))
		{
			param_.initPos = transform_.pos;
		}
		ImGui::SameLine();
		if (ImGui::Button("Warp to Init Pos"))
		{
			transform_.pos = param_.initPos;
			transform_.Update();
		}
		if (ImGui::DragFloat3("Scale", &param_.initScale.x, 0.05f, 0.01f, 20.0f)) changed = true;
		if (ImGui::DragFloat("Local Angle Y", &param_.initAngleY, 1.0f, -360.0f, 360.0f)) changed = true;
	}

	// 3. Collider (線分・カプセル)
	if (ImGui::CollapsingHeader("Collider", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::Text("[Ground Line Collider]");
		if (ImGui::DragFloat3("Line Start", &param_.colLineStart.x, 1.0f)) changed = true;
		if (ImGui::DragFloat3("Line End", &param_.colLineEnd.x, 1.0f)) changed = true;

		ImGui::Separator();
		ImGui::Text("[Body Capsule Collider]");
		if (ImGui::DragFloat3("Capsule Top", &param_.colCapsuleTop.x, 1.0f)) changed = true;
		if (ImGui::DragFloat3("Capsule Down", &param_.colCapsuleDown.x, 1.0f)) changed = true;
		if (ImGui::DragFloat("Capsule Radius", &param_.colCapsuleRadius, 0.5f, 1.0f, 300.0f)) changed = true;
	}

	// 4. Status (HP・スタミナ・回復・無敵)
	if (ImGui::CollapsingHeader("Status (HP / Stamina / Recovery)"))
	{
		if (ImGui::DragFloat("Max HP", &param_.maxHp, 1.0f, 1.0f, 2000.0f)) changed = true;
		if (ImGui::DragFloat("Max Stamina", &param_.maxStamina, 1.0f, 1.0f, 2000.0f)) changed = true;
		ImGui::DragFloat("Stamina Recovery Speed", &param_.recoveryStSpeed, 0.5f, 0.0f, 200.0f);
		ImGui::DragFloat("Stamina Cost (Dash)", &param_.consumptionStFastRun, 0.5f, 0.0f, 200.0f);
		ImGui::DragFloat("Stamina Cost (Evasion)", &param_.consumptionStEvasion, 0.5f, 0.0f, 200.0f);
		ImGui::DragFloat("Stamina Cost (Attack)", &param_.consumptionStAttack, 0.5f, 0.0f, 200.0f);
		ImGui::DragFloat("Stamina Exhaustion Time", &param_.staminaExhaustionTime, 0.05f, 0.0f, 10.0f);
		ImGui::DragFloat("Invincible Time", &param_.invincibleTime, 0.05f, 0.0f, 20.0f);
		ImGui::DragFloat("Heal HP Amount", &param_.healHpAmount, 1.0f, 0.0f, 500.0f);
		ImGui::InputInt("Initial Bottle Count", &param_.initialRecoveryBottleCount);
		ImGui::InputInt("Consume Bottle Count", &param_.consumeRecoveryBottleCount);
	}

	// 5. Movement (移動・回避・補間速度)
	if (ImGui::CollapsingHeader("Movement"))
	{
		ImGui::DragFloat("Walk Speed", &param_.speedMove, 0.1f, 0.0f, 100.0f);
		ImGui::DragFloat("Dash Speed", &param_.speedDash, 0.1f, 0.0f, 100.0f);
		ImGui::DragFloat("Evasion Speed", &param_.speedEvasion, 0.1f, 0.0f, 100.0f);
		ImGui::DragFloat("Move Deceleration", &param_.movePowDecrease, 0.01f, 0.01f, 1.0f);
		ImGui::DragFloat("Avoidance Angle Y", &param_.avoidanceAngleAxisY, 1.0f, -360.0f, 360.0f);
		ImGui::DragFloat("Target Rot Slerp Rate", &param_.rotTargetSlerpRate, 0.01f, 0.01f, 1.0f);
		ImGui::DragFloat("Attack Step Lerp Rate", &param_.attackStepLerpRate, 0.01f, 0.01f, 1.0f);
	}

	// 6. Combat (被ダメージ・押し出し)
	if (ImGui::CollapsingHeader("Combat / Damage"))
	{
		ImGui::DragFloat("Damage from Enemy Body", &param_.damageEnemyBody, 1.0f, 0.0f, 500.0f);
		ImGui::DragFloat("Damage from Enemy Weapon", &param_.damageEnemyWeapon, 1.0f, 0.0f, 500.0f);
		ImGui::InputInt("Push Back Power", &param_.pushBackPower);
	}

	// 7. Animation Speed (アニメーション再生速度)
	if (ImGui::CollapsingHeader("Animation Speed"))
	{
		if (ImGui::DragFloat("Idle Speed", &param_.animSpeedIdle, 0.5f, 1.0f, 200.0f)) changed = true;
		if (ImGui::DragFloat("Walk (Run) Speed", &param_.animSpeedRun, 0.5f, 1.0f, 200.0f)) changed = true;
		if (ImGui::DragFloat("Fast Run Speed", &param_.animSpeedFastRun, 0.5f, 1.0f, 200.0f)) changed = true;
		if (ImGui::DragFloat("Attack 1 Speed", &param_.animSpeedAttack1, 0.5f, 1.0f, 200.0f)) changed = true;
		if (ImGui::DragFloat("Attack 2 Speed", &param_.animSpeedAttack2, 0.5f, 1.0f, 200.0f)) changed = true;
		if (ImGui::DragFloat("Attack 3 Speed", &param_.animSpeedAttack3, 0.5f, 1.0f, 200.0f)) changed = true;
		if (ImGui::DragFloat("Evasion Speed", &param_.animSpeedEvasion, 0.5f, 1.0f, 200.0f)) changed = true;
		if (ImGui::DragFloat("Down Speed", &param_.animSpeedDown, 0.5f, 1.0f, 200.0f)) changed = true;
		if (ImGui::DragFloat("Up Speed", &param_.animSpeedUp, 0.5f, 1.0f, 200.0f)) changed = true;
		if (ImGui::DragFloat("Recovery Speed", &param_.animSpeedRecovery, 0.5f, 1.0f, 200.0f)) changed = true;
	}

	// 8. Attack Combo (コンボ1?3)
	if (ImGui::CollapsingHeader("Attack Combo"))
	{
		const char* comboLabels[] = { "Combo 1", "Combo 2", "Combo 3" };
		for (int i = 0; i < 3; ++i)
		{
			auto key = static_cast<STATE_ATTACK_COMBO>(i);
			if (atkComboData_.count(key) == 0) continue;
			auto& c = atkComboData_.at(key);

			if (ImGui::TreeNode(comboLabels[i]))
			{
				ImGui::DragFloat("Input Start Step", &c.stepInputStart, 0.5f, 0.0f, 200.0f);
				ImGui::DragFloat("Input End Step", &c.stepInputEnd, 0.5f, 0.0f, 200.0f);
				ImGui::DragFloat("Collider Start Step", &c.stepCollisionStart, 0.5f, 0.0f, 200.0f);
				ImGui::DragFloat("Collider End Step", &c.stepCollisionEnd, 0.5f, 0.0f, 200.0f);
				ImGui::DragFloat("Interrupt Step", &c.stepInterrupt, 0.5f, 0.0f, 200.0f);
				ImGui::DragFloat("Step Move Speed", &c.moveSpeed, 0.5f, 0.0f, 100.0f);
				ImGui::TreePop();
			}
		}
	}

	// 9. Effect & Bone Frame
	if (ImGui::CollapsingHeader("Effect / Sound / Bone Lock"))
	{
		ImGui::SliderInt("SE Volume", &param_.seVolume, 0, 100);
		ImGui::DragFloat3("Blood Effect Scale", &param_.effectBloodScale.x, 0.2f, 0.1f, 50.0f);
		ImGui::DragFloat("Recovery Effect Scale", &param_.effectRecoveryScale, 0.2f, 0.1f, 50.0f);
		ImGui::DragFloat("Recovery Effect Offset Y", &param_.effectRecoveryOffsetY, 1.0f, -200.0f, 500.0f);
		ImGui::DragFloat("Attack SE Trigger Step", &param_.attackSeTriggerStep, 0.5f, 0.0f, 100.0f);
		ImGui::DragFloat("Recovery Trigger Step", &param_.recoveryTriggerStep, 0.5f, 0.0f, 100.0f);
		ImGui::InputInt("Weapon Attach Frame No", &param_.weaponAttachFrameNo);
		ImGui::InputInt("Hit Effect Frame No", &param_.hitEffectFrameNo);
		ImGui::DragFloat3("Root Lock Local Pos", &param_.lockPos1.x, 0.5f);
		ImGui::InputInt("Root Lock Frame No", &param_.lockFrameNo);
	}

	if (changed)
	{
		ApplyParam(false);
	}
#endif
}

void Player::InitWeapon(void)
{
	// 武器
	auto weaponSaw = std::make_unique<WeaponSaw>(transform_, param_.weaponAttachFrameNo);
	weapons_.push_back(std::move(weaponSaw));
	for (const auto& weapon : weapons_) {
		weapon->Load();
	}
}

void Player::InitSound(void)
{
	// サウンド登録
	int seHandlId = -1;
	// 歩きSE
	seHandlId = resMng_.Load(SE_PLAYER_WAKE_NAME).handleId_;
	sngMng_.Add(SoundManager::SOUND_ID::PLAYER_WAKE, seHandlId, param_.seVolume);
	// 走りSE
	seHandlId = resMng_.Load(SE_PLAYER_RUN_NAME).handleId_;
	sngMng_.Add(SoundManager::SOUND_ID::PLAYER_RAN, seHandlId, param_.seVolume);
	// 回復SE
	seHandlId = resMng_.Load(SE_PLAYER_RECOVERY_NAME).handleId_;
	sngMng_.Add(SoundManager::SOUND_ID::PLAYER_RAN, seHandlId, param_.seVolume);
	// 回避SE
	seHandlId = resMng_.Load(SE_PLAYER_EVASION_NAME).handleId_;
	sngMng_.Add(SoundManager::SOUND_ID::PLAYER_RAN, seHandlId, param_.seVolume);
	// 攻撃SE
	seHandlId = resMng_.Load(SE_PLAYER_WEAPON_1_NAME).handleId_;
	sngMng_.Add(SoundManager::SOUND_ID::PLAYER_WEPON_SE1, seHandlId, param_.seVolume);
	// 被ダメージSE
	seHandlId = resMng_.Load(SE_PLAYER_DAMAGE_NAME).handleId_;
	sngMng_.Add(SoundManager::SOUND_ID::PLAYER_RAN, seHandlId, param_.seVolume);
}

void Player::InitState(void)
{
	// 配列初期化
	states_.clear();

	// 各ステートの登録
	// 待機状態
	states_.emplace(STATE::IDLE, std::make_unique<PlayerIdleState>());
	// 移動状態
	states_.emplace(STATE::MOVE, std::make_unique<PlayerMoveState>());
	// 攻撃状態
	states_.emplace(STATE::ATTACK, std::make_unique<PlayerAttackState>());
	// 回避状態
	states_.emplace(STATE::EVASION, std::make_unique<PlayerEvasionState>());
	// ダメージ状態
	states_.emplace(STATE::DAMAGE, std::make_unique<PlayerDamageState>());
	// 回復状態
	states_.emplace(STATE::RECOVERY, std::make_unique<PlayerRecoveryState>());

	// ステートを設定
	currentState_ = states_.at(state_).get();
	currentState_->StartState(*this);
}

void Player::UpdateProcess(void)
{
	// 状態ごとの更新
	currentState_->Update(*this);

	// 無敵時間処理
	ProcessVinclible();

	// スタミナ回復処理
	ProcessStaminaRecovery();

	// 武器処理
	for (const auto& weapon : weapons_)
	{
		weapon->Update();
	}

	// 移動量の減速処理
	movePow_ = AsoUtility::Lerp(
		movePow_, AsoUtility::VECTOR_ZERO, param_.movePowDecrease);

	// アニメーションの移動量を無効
	SetFrameUserLocalPos(param_.lockPos1, param_.lockFrameNo);
}

void Player::UpdateProcessPost(void)
{
	if (targetPos_ == nullptr) return;

	// ターゲットに向かって回転
	VECTOR targetDir = GetTargetDir();
	float targetAngleY = atan2f(targetDir.x, targetDir.z);
	Quaternion targetRot =
		Quaternion::AngleAxis(targetAngleY, AsoUtility::AXIS_Y);
	transform_.quaRot =
		Quaternion::Slerp(transform_.quaRot, targetRot, param_.rotTargetSlerpRate);
}

void Player::UpdateHitCollider(void)
{
	if (GetIsInvincible()) return;

	int capsuleType = static_cast<int>(ColliderBase::SHAPE::CAPSULE);
	if (ownColliders_.count(capsuleType) == 0) return;

	// 受けるダメージ量を保持する変数
	float damage = 0.0f;

	for (const auto& vec : ownColliders_.at(capsuleType))
	{
		auto playerCapsule = std::dynamic_pointer_cast<ColliderCapsule>(vec);
		if (!playerCapsule || !playerCapsule->GetIsCollier()) continue;

		const auto& hitList = playerCapsule->GetHitColliders();

		for (const auto& hitCol : hitList)
		{
			auto targetCol = hitCol.lock();
			if (!targetCol || !targetCol->GetIsCollier()) continue;

			auto shape = targetCol->GetShape();
			auto tag = targetCol->GetTag();

			// 敵本体との当たり判定
			if (tag == ColliderBase::TAG::ENEMY
				&& shape == ColliderBase::SHAPE::CAPSULE)
			{
				auto enemyCapsule = std::dynamic_pointer_cast<ColliderCapsule>(targetCol);

				transform_.pos =
					VAdd(transform_.pos,
						CollisionUtility::CalcPushCapsuleCapsule(
							*playerCapsule, *enemyCapsule));

				if (!enemyCapsule->GetIsAction()) continue;

				// 複数同時に当たった場合は大きい方のダメージを優先
				damage = param_.damageEnemyBody;
				continue;
			}

			// タグが敵武器でないときは処理しない
			if (tag != ColliderBase::TAG::ENEMY_WEPON) continue;

			// 壁による遮蔽チェック
			if (targetCol->GetFollow() != nullptr
				&& playerCapsule->GetFollow() != nullptr)
			{
				if (CollisionManager::GetInstance().CheckWallOcclusion(
					targetCol->GetFollow()->pos,
					playerCapsule->GetFollow()->pos)) continue;
			}

			// 敵武器との当たり判定
			if (shape == ColliderBase::SHAPE::CAPSULE
				|| shape == ColliderBase::SHAPE::SPHERE)
			{
				damage = param_.damageEnemyWeapon;
			}
		}
	}

	if (damage > 0.0f)
	{
		hp_ -= damage;
		ChangeState(STATE::DAMAGE);
	}
}

void Player::ChangeState(STATE state)
{
	// 同じ状態なら遷移しない
	if (state_ == state) return;

	// 現在の状態の終了処理
	currentState_->EndState(*this);

	// 次の状態の開始処理
	state_ = state;
	currentState_ = states_.at(state_).get();
	currentState_->StartState(*this);
}

VECTOR Player::GetInputMoveDir(void)
{
	VECTOR dir = AsoUtility::VECTOR_ZERO;
	auto& ins = InputManager::GetInstance();

	// ゲームパッドの入力を取得
	InputManager::JOYPAD_IN_STATE padState =
		ins.GetJPadInputState(InputManager::JOYPAD_NO::PAD1);
	VECTOR padDir = ins.GetDirectionXZAKey(padState.AKeyLX, padState.AKeyLY);

	// 両対応：ゲームパッド優先、無ければキーボード
	if (!AsoUtility::EqualsVZero(padDir))
	{
		dir = padDir;
	}
	else
	{
		// キーボード入力
		bool isUp = ins.IsNew(KEY_INPUT_W);
		bool isDown = ins.IsNew(KEY_INPUT_S);
		bool isLeft = ins.IsNew(KEY_INPUT_A);
		bool isRight = ins.IsNew(KEY_INPUT_D);

		// 入力方向の計算
		if (isUp) { dir = AsoUtility::DIR_F; }
		if (isLeft) { dir = AsoUtility::DIR_L; }
		if (isDown) { dir = AsoUtility::DIR_B; }
		if (isRight) { dir = AsoUtility::DIR_R; }

		// 斜め入力の計算
		if (isUp && isLeft) { dir = VAdd(AsoUtility::DIR_F, AsoUtility::DIR_L); }
		if (isUp && isRight) { dir = VAdd(AsoUtility::DIR_F, AsoUtility::DIR_R); }
		if (isDown && isLeft) { dir = VAdd(AsoUtility::DIR_B, AsoUtility::DIR_L); }
		if (isDown && isRight) { dir = VAdd(AsoUtility::DIR_B, AsoUtility::DIR_R); }

		// 斜め入力の正規化
		if (!AsoUtility::EqualsVZero(dir)) {
			dir = VNorm(dir);
		}
	}

	// 入力がない場合はゼロベクトルを返す
	if (AsoUtility::EqualsVZero(dir))
	{
		return AsoUtility::VECTOR_ZERO;
	}

	// ターゲットまたはカメラ基準のワールド方向に変換
	VECTOR worldDir = AsoUtility::VECTOR_ZERO;

	if (targetPos_ != nullptr)
	{
		// ターゲットがいる場合
		VECTOR targetDir = GetTargetDir();
		float targetAngleY = atan2f(targetDir.x, targetDir.z);
		Quaternion targetRot = Quaternion::AngleAxis(targetAngleY, AsoUtility::AXIS_Y);
		worldDir = Quaternion::PosAxis(targetRot, dir);
	}
	else
	{
		// カメラ方向を基準に変換
		Quaternion cameraRot = scnMng_.GetCamera()->GetQuaRotY();
		worldDir = Quaternion::PosAxis(cameraRot, dir);
	}

	return worldDir;
}

bool Player::GetInputDash(void)
{
	// ダッシュ入力判定
	auto& ins = InputManager::GetInstance();
	bool isR = ins.IsNew(KEY_INPUT_LSHIFT) || ins.IsPadBtnNew(
		InputManager::JOYPAD_NO::PAD1, InputManager::JOYPAD_BTN::DOWN);

	return isR;
}

bool Player::GetInputAttack(void)
{
	// スタミナがないときは処理しない
	if (stamina_ <= 0) return false;

#ifdef _DEBUG
	// ImGui操作中はマウスクリックによる攻撃を無効化する
	if (ImGui::GetIO().WantCaptureMouse)
	{
		return InputManager::GetInstance().IsPadBtnTrgDown(
			InputManager::JOYPAD_NO::PAD1, InputManager::JOYPAD_BTN::R_TRIGGER);
	}
#endif

	// 攻撃入力判定
	auto& ins = InputManager::GetInstance();
	bool isAttackInput = ins.IsClickMouseLeft() || ins.IsPadBtnTrgDown(
		InputManager::JOYPAD_NO::PAD1, InputManager::JOYPAD_BTN::R_TRIGGER);

	return isAttackInput;
}

bool Player::GetInputEvasion(void)
{
	// スタミナがないときは処理しない
	if (stamina_ <= 0) return false;

	// 回避入力判定
	auto& ins = InputManager::GetInstance();
	bool isEvasion = ins.IsTrgDown(KEY_INPUT_SPACE) || ins.IsPadBtnNew(
		InputManager::JOYPAD_NO::PAD1, InputManager::JOYPAD_BTN::RIGHT);

	return isEvasion;
}

bool Player::GetInputRecovery(void)
{
	// 回復瓶がないときは処理しない
	if (recoveryBottleCount_ <= 0) return false;

	// 回復入力判定
	auto& ins = InputManager::GetInstance();
	bool isRecovery = ins.IsTrgDown(KEY_INPUT_R) || ins.IsPadBtnNew(
		InputManager::JOYPAD_NO::PAD1, InputManager::JOYPAD_BTN::LEFT);

	return isRecovery;
}

Player::STATE Player::GetNextStateByInput(void)
{
	// 入力と遷移先の対応表(上にあるものほど優先)
	struct InputToState
	{
		bool (Player::* isInput)(void);
		STATE nextState;
	};
	static const InputToState INPUT_TABLE[] =
	{
		{ &Player::GetInputAttack,   STATE::ATTACK },
		{ &Player::GetInputEvasion,  STATE::EVASION },
		{ &Player::GetInputRecovery, STATE::RECOVERY },
	};

	// アクション入力
	for (const auto& entry : INPUT_TABLE)
	{
		if ((this->*entry.isInput)())
		{
			return entry.nextState;
		}
	}

	// 移動入力
	if (!AsoUtility::EqualsVZero(GetInputMoveDir()))
	{
		return STATE::MOVE;
	}

	// 入力なし
	return STATE::IDLE;
}

void Player::ProcessVinclible(void)
{
	// 無敵時間が0以下の場合は処理を行わない
	if (invincibleTimer_ <= 0.0f) return;

	// 無敵時間の更新
	invincibleTimer_ -= scnMng_.GetDeltaTime();
}

void Player::ProcessStaminaRecovery(void)
{
	// スタミナが最大値に達している場合は処理を行わない
	if (stamina_ >= param_.maxStamina)
	{
		stamina_ = param_.maxStamina;
		return;
	}

	if (state_ != STATE::IDLE
		&& state_ != STATE::MOVE
		&& state_ != STATE::RECOVERY) return;

	// スタミナ回復処理
	stamina_ += param_.recoveryStSpeed * scnMng_.GetDeltaTime();
}
