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
#include "../../Weapon/WeaponBase.h"
#include "../../Weapon/WeaponManager.h"
#include "../../UI/UiSt.h"
#include "../../UI/UiHp.h"
#include "../../UI/UiRecovery.h"
#include "State/PlayerMoveState.h"
#include "State/PlayerIdleState.h"
#include "State/PlayerAttackState.h"
#include "State/PlayerEvasionState.h"
#include "State/PlayerDamageState.h"
#include "State/PlayerRecoveryState.h"
#include "Player.h"

namespace
{
	constexpr const wchar_t* MODEL_PLAYER_NAME = L"ModelPlayer";
	constexpr const wchar_t* ANIM_PLAYER_IDLE_NAME = L"AnimationPlayerIdle";
	constexpr const wchar_t* ANIM_PLAYER_MOVE_NAME = L"AnimationPlayerMove";
	constexpr const wchar_t* ANIM_PLAYER_ATTACK_1_NAME = L"AnimationPlayerAttack1";
	constexpr const wchar_t* ANIM_PLAYER_ATTACK_2_NAME = L"AnimationPlayerAttack2";
	constexpr const wchar_t* ANIM_PLAYER_ATTACK_3_NAME = L"AnimationPlayerAttack3";
	constexpr const wchar_t* ANIM_PLAYER_EVASION_NAME = L"AnimationPlayerEvasion";
	constexpr const wchar_t* ANIM_PLAYER_DOWN_NAME = L"AnimationPlayerDown";
	constexpr const wchar_t* ANIM_PLAYER_UP_NAME = L"AnimationPlayerUp";
	constexpr const wchar_t* ANIM_PLAYER_RECOVERY_NAME = L"AnimationPlayerRecovery";
	constexpr const wchar_t* JSON_PLAYER_DATA_NAME = L"PlayerData";

	constexpr const wchar_t* SE_PLAYER_WAKE_NAME = L"SePlayerWake";
	constexpr const wchar_t* SE_PLAYER_RUN_NAME = L"SePlayerRun";
	constexpr const wchar_t* SE_PLAYER_RECOVERY_NAME = L"SePlayerRecovery";
	constexpr const wchar_t* SE_PLAYER_EVASION_NAME = L"SePlayerEvasion";
	constexpr const wchar_t* SE_PLAYER_WEAPON_1_NAME = L"SePlayerWeapon1";
	constexpr const wchar_t* SE_PLAYER_DAMAGE_NAME = L"SePlayerDamage";
}

Player::Player(void)
	: CharactorBase()
	, animLockPos_(AsoUtility::VECTOR_ZERO)
	, effType_(EFFECT::NONE)
	, invincibleTimer_(0.0f)
	, isComboNext_(false)
	, stateAtkCombo_(STATE_ATTACK_COMBO::COMBO_1)
	, state_(static_cast<int>(STATE::IDLE))
	, stamina_(100.0f)
	, stCoolTime_(0.0f)
	, currentState_(nullptr)
	, recoveryBottleCount_(6)
{
}

Player::~Player(void) {}

Json& Player::GetData(void) const
{
	return resMng_.GetJson(JSON_PLAYER_DATA_NAME);
}

void Player::Draw(void)
{
	CharactorBase::Draw();
}

void Player::Release(void)
{
#ifdef _DEBUG
	EditorManager::GetInstance().UnregisterObject("Player");
#endif
	CharactorBase::Release();
}

void Player::InitLoad(void)
{
	const auto& status = GetData()["status"];
	hp_ = status["maxHp"];
	stamina_ = status["maxStamina"];
	recoveryBottleCount_ = status["initialRecoveryBottleCount"];

	// プレイヤーモデル
	transform_.SetModel(resMng_.LoadModelDuplicate(MODEL_PLAYER_NAME));

	InitWeapon();
	InitSound();

	// プレイヤーUI
	auto& ui = UiManager::GetInstance();
	ui.Add(std::make_unique<UiHp>(&hp_, Application::ASPECT_RATIO_X / 4.0f, 20.0f, 2.9f, 4.0f, 0.7f));
	ui.Add(std::make_unique<UiSt>(&stamina_, Application::ASPECT_RATIO_X / 4.0f, 60.0f, 2.9f, 4.0f, 0.7f));
}

void Player::InitTransform(void)
{
	const auto& trans = GetData()["transform"];
	transform_.pos = trans["position"];
	transform_.scl = VScale(AsoUtility::VECTOR_ONE, trans["scale"]);
	transform_.quaRot = Quaternion::Identity();
	transform_.quaRotLocal = Quaternion::AngleAxis(
		AsoUtility::Deg2RadF(trans["angleY"].get<float>()), AsoUtility::AXIS_Y);
	transform_.Update();
}

void Player::InitCollider(void)
{
	const auto& col = GetData()["collider"];

	// 線分コライダ
	std::vector<ColliderBase::TAG> lineTags = { ColliderBase::TAG::STAGE };
	auto colLine = std::make_shared<ColliderLine>(
		ColliderBase::TAG::GROUND,
		lineTags,
		&transform_,
		col["line"]["startPos"],
		col["line"]["endPos"]);
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
		col["capsule"]["topPos"],
		col["capsule"]["downPos"],
		col["capsule"]["radius"].get<float>());
	ownColliders_[static_cast<int>(ColliderBase::SHAPE::CAPSULE)].push_back(colCapsule);
	colMng_.AddCollider(colCapsule);
}

void Player::InitAnimation(void)
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

	addAnim(ANIM_TYPE::IDLE,     "idle",     ANIM_PLAYER_IDLE_NAME);
	addAnim(ANIM_TYPE::RUN,      "run",      ANIM_PLAYER_MOVE_NAME);
	addAnim(ANIM_TYPE::FAST_RUN, "fastRun",  ANIM_PLAYER_MOVE_NAME);
	addAnim(ANIM_TYPE::ATTACK_1, "attack1",  ANIM_PLAYER_ATTACK_1_NAME);
	addAnim(ANIM_TYPE::ATTACK_2, "attack2",  ANIM_PLAYER_ATTACK_2_NAME);
	addAnim(ANIM_TYPE::ATTACK_3, "attack3",  ANIM_PLAYER_ATTACK_3_NAME);
	addAnim(ANIM_TYPE::EVASION,  "evasion",  ANIM_PLAYER_EVASION_NAME);
	addAnim(ANIM_TYPE::DOWN,     "down",     ANIM_PLAYER_DOWN_NAME);
	addAnim(ANIM_TYPE::UP,       "up",       ANIM_PLAYER_UP_NAME);
	addAnim(ANIM_TYPE::RECOVERY, "recovery", ANIM_PLAYER_RECOVERY_NAME);

	animationController_->Play(static_cast<int>(ANIM_TYPE::IDLE));
}

void Player::InitPost(void)
{
	weapons_->LoadEnd();

	effType_ = EFFECT::NONE;
	effect_ = std::make_unique<EffectController>();
	effect_->Add(static_cast<int>(EFFECT::BLOOD), (Application::PATH_EFFECT + L"Blood.efkefc"));
	effect_->Add(static_cast<int>(EFFECT::HP_ABSOLUTE), (Application::PATH_EFFECT + L"Absolute.efkefc"));

	LoadComboFromJson();
	stateAtkCombo_ = STATE_ATTACK_COMBO::COMBO_1;
	isComboNext_ = false;
	InitState();

#ifdef _DEBUG
	EditorManager::GetInstance().RegisterObject({
		"Player",
		JSON_PLAYER_DATA_NAME,
		[this]() { this->ApplyJson(); },
		[this]() { this->UpdateGui(); }
	});
#endif
}

void Player::LoadComboFromJson(void)
{
	atkComboData_.clear();
	const auto& playerCombo = GetData()["combo"];
	int cont = 0;
	for (const auto& combo : playerCombo)
	{
		ATTACK_COMBO data = {
			static_cast<ANIM_TYPE>(combo.at("animType").get<int>()),
			combo.at("inputStart").get<float>(),
			combo.at("inputEnd").get<float>(),
			combo.at("colStart").get<float>(),
			combo.at("colEnd").get<float>(),
			combo.at("stepInterrupt").get<float>(),
			combo.at("moveSpeed").get<float>(),
			static_cast<STATE_ATTACK_COMBO>(combo.at("nextCombo").get<int>()),
		};
		atkComboData_.emplace(static_cast<STATE_ATTACK_COMBO>(cont++), data);
	}
}

void Player::ApplyJson(void)
{
	const auto& data = GetData();

	// Transform反映
	transform_.scl = VScale(AsoUtility::VECTOR_ONE, data["scale"]);
	transform_.quaRotLocal = Quaternion::AngleAxis(
		AsoUtility::Deg2RadF(data["transform"]["angleY"].get<float>()), AsoUtility::AXIS_Y);
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

		applyAnim(ANIM_TYPE::IDLE,     "idle");
		applyAnim(ANIM_TYPE::RUN,      "run");
		applyAnim(ANIM_TYPE::FAST_RUN, "fastRun");
		applyAnim(ANIM_TYPE::ATTACK_1, "attack1");
		applyAnim(ANIM_TYPE::ATTACK_2, "attack2");
		applyAnim(ANIM_TYPE::ATTACK_3, "attack3");
		applyAnim(ANIM_TYPE::EVASION,  "evasion");
		applyAnim(ANIM_TYPE::DOWN,     "down");
		applyAnim(ANIM_TYPE::UP,       "up");
		applyAnim(ANIM_TYPE::RECOVERY, "recovery");
	}

	// コンボ反映
	LoadComboFromJson();
}

void Player::UpdateGui(void)
{
#ifdef _DEBUG
	if (ImGui::CollapsingHeader("Runtime State", ImGuiTreeNodeFlags_DefaultOpen))
	{
		float maxHp = GetData()["status"]["maxHp"];
		float maxSt = GetData()["status"]["maxStamina"];
		if (ImGui::DragFloat3("Current Pos", &transform_.pos.x, 1.0f)) transform_.Update();
		ImGui::SliderFloat("Current HP", &hp_, 0.0f, maxHp);
		ImGui::SliderFloat("Current Stamina", &stamina_, 0.0f, maxSt);
		if (animationController_)
		{
			ImGui::Text("Anim Type: %d  Blending: %s (%.2f)",
				animationController_->GetPlayType(),
				animationController_->IsBlending() ? "ON" : "OFF",
				animationController_->GetBlendRate());
		}
		if (ImGui::Button("Warp to Init Pos"))
		{
			transform_.pos = GetData()["transform"]["position"];
			transform_.Update();
		}
		ImGui::SameLine();
		if (ImGui::Button("Set Current Pos -> JSON"))
		{
			GetData()["transform"]["position"] = transform_.pos;
		}
	}
#endif
}

void Player::InitWeapon(void)
{
	int attachFrame = GetData()["effectAndSound"]["weaponAttachFrameNo"];
	weapons_->Add(WeaponBase::WEAPON_TYPE::SAW, transform_, attachFrame);
	weapons_->Load();
}

void Player::InitSound(void)
{
	int vol = GetData()["effectAndSound"]["seVolume"];
	sngMng_.Add(SoundManager::SOUND_ID::PLAYER_WAKE, resMng_.Load(SE_PLAYER_WAKE_NAME).handleId_, vol);
	sngMng_.Add(SoundManager::SOUND_ID::PLAYER_RAN, resMng_.Load(SE_PLAYER_RUN_NAME).handleId_, vol);
	sngMng_.Add(SoundManager::SOUND_ID::PLAYER_RAN, resMng_.Load(SE_PLAYER_RECOVERY_NAME).handleId_, vol);
	sngMng_.Add(SoundManager::SOUND_ID::PLAYER_RAN, resMng_.Load(SE_PLAYER_EVASION_NAME).handleId_, vol);
	sngMng_.Add(SoundManager::SOUND_ID::PLAYER_WEPON_SE1, resMng_.Load(SE_PLAYER_WEAPON_1_NAME).handleId_, vol);
	sngMng_.Add(SoundManager::SOUND_ID::PLAYER_RAN, resMng_.Load(SE_PLAYER_DAMAGE_NAME).handleId_, vol);
}

void Player::InitState(void)
{
	states_.clear();
	states_.emplace(static_cast<int>(STATE::IDLE), std::make_unique<PlayerIdleState>());
	states_.emplace(static_cast<int>(STATE::MOVE), std::make_unique<PlayerMoveState>());
	states_.emplace(static_cast<int>(STATE::ATTACK), std::make_unique<PlayerAttackState>());
	states_.emplace(static_cast<int>(STATE::EVASION), std::make_unique<PlayerEvasionState>());
	states_.emplace(static_cast<int>(STATE::DAMAGE), std::make_unique<PlayerDamageState>());
	states_.emplace(static_cast<int>(STATE::RECOVERY), std::make_unique<PlayerRecoveryState>());

	currentState_ = states_.at(state_).get();
	currentState_->StartState(*this);
}

void Player::UpdateProcess(void)
{
	currentState_->Update(*this);
	ProcessVinclible();
	ProcessStaminaRecovery();

	const auto& data = GetData();
	movePow_ = AsoUtility::Lerp(
		movePow_, AsoUtility::VECTOR_ZERO, data["movement"]["movePowDecrease"].get<float>());

	SetFrameUserLocalPos(
		data["effectAndSound"]["lockPos1"],
		data["effectAndSound"]["lockFrameNo"].get<int>());
}

void Player::UpdateProcessPost(void)
{
	if (targetPos_ == nullptr) return;

	VECTOR targetDir = GetTargetDir();
	float targetAngleY = atan2f(targetDir.x, targetDir.z);
	Quaternion targetRot = Quaternion::AngleAxis(targetAngleY, AsoUtility::AXIS_Y);
	transform_.quaRot = Quaternion::Slerp(
		transform_.quaRot, targetRot, GetData()["movement"]["rotTargetSlerpRate"].get<float>());
}

void Player::UpdateHitCollider(void)
{
	if (GetIsInvincible()) return;

	int capsuleType = static_cast<int>(ColliderBase::SHAPE::CAPSULE);
	if (ownColliders_.count(capsuleType) == 0) return;

	float damage = 0.0f;
	const auto& combat = GetData()["combat"];

	for (const auto& vec : ownColliders_.at(capsuleType))
	{
		auto playerCapsule = std::dynamic_pointer_cast<ColliderCapsule>(vec);
		if (!playerCapsule || !playerCapsule->GetIsCollier()) continue;

		for (const auto& hitCol : playerCapsule->GetHitColliders())
		{
			auto targetCol = hitCol.lock();
			if (!targetCol || !targetCol->GetIsCollier()) continue;

			auto shape = targetCol->GetShape();
			auto tag = targetCol->GetTag();

			if (tag == ColliderBase::TAG::ENEMY && shape == ColliderBase::SHAPE::CAPSULE)
			{
				auto enemyCapsule = std::dynamic_pointer_cast<ColliderCapsule>(targetCol);
				transform_.pos = VAdd(transform_.pos,
					CollisionUtility::CalcPushCapsuleCapsule(*playerCapsule, *enemyCapsule));

				if (enemyCapsule->GetIsAction())
				{
					damage = combat["damageEnemyBody"];
				}
				continue;
			}

			if (tag != ColliderBase::TAG::ENEMY_WEPON) continue;

			if (targetCol->GetFollow() != nullptr && playerCapsule->GetFollow() != nullptr)
			{
				if (CollisionManager::GetInstance().CheckWallOcclusion(
					targetCol->GetFollow()->pos, playerCapsule->GetFollow()->pos)) continue;
			}

			if (shape == ColliderBase::SHAPE::CAPSULE || shape == ColliderBase::SHAPE::SPHERE)
			{
				damage = combat["damageEnemyWeapon"];
			}
		}
	}

	if (damage > 0.0f)
	{
		hp_ -= damage;
		ChangeState(static_cast<int>(STATE::DAMAGE));
	}
}

void Player::ChangeState(int state)
{
	if (state_ == state) return;
	currentState_->EndState(*this);
	state_ = state;
	currentState_ = states_.at(state_).get();
	currentState_->StartState(*this);
}

VECTOR Player::GetInputMoveDir(void)
{
	VECTOR dir = AsoUtility::VECTOR_ZERO;
	auto& ins = InputManager::GetInstance();

	InputManager::JOYPAD_IN_STATE padState =
		ins.GetJPadInputState(InputManager::JOYPAD_NO::PAD1);
	VECTOR padDir = ins.GetDirectionXZAKey(padState.AKeyLX, padState.AKeyLY);

	if (!AsoUtility::EqualsVZero(padDir))
	{
		dir = padDir;
	}
	else
	{
		bool isUp = ins.IsNew(KEY_INPUT_W);
		bool isDown = ins.IsNew(KEY_INPUT_S);
		bool isLeft = ins.IsNew(KEY_INPUT_A);
		bool isRight = ins.IsNew(KEY_INPUT_D);

		if (isUp) { dir = AsoUtility::DIR_F; }
		if (isLeft) { dir = AsoUtility::DIR_L; }
		if (isDown) { dir = AsoUtility::DIR_B; }
		if (isRight) { dir = AsoUtility::DIR_R; }

		if (isUp && isLeft) { dir = VAdd(AsoUtility::DIR_F, AsoUtility::DIR_L); }
		if (isUp && isRight) { dir = VAdd(AsoUtility::DIR_F, AsoUtility::DIR_R); }
		if (isDown && isLeft) { dir = VAdd(AsoUtility::DIR_B, AsoUtility::DIR_L); }
		if (isDown && isRight) { dir = VAdd(AsoUtility::DIR_B, AsoUtility::DIR_R); }

		if (!AsoUtility::EqualsVZero(dir)) {
			dir = VNorm(dir);
		}
	}

	if (AsoUtility::EqualsVZero(dir))
	{
		return AsoUtility::VECTOR_ZERO;
	}

	if (targetPos_ != nullptr)
	{
		VECTOR targetDir = GetTargetDir();
		float targetAngleY = atan2f(targetDir.x, targetDir.z);
		Quaternion targetRot = Quaternion::AngleAxis(targetAngleY, AsoUtility::AXIS_Y);
		return Quaternion::PosAxis(targetRot, dir);
	}

	Quaternion cameraRot = scnMng_.GetCamera()->GetQuaRotY();
	return Quaternion::PosAxis(cameraRot, dir);
}

bool Player::GetInputDash(void)
{
	auto& ins = InputManager::GetInstance();
	return ins.IsNew(KEY_INPUT_LSHIFT) || ins.IsPadBtnNew(
		InputManager::JOYPAD_NO::PAD1, InputManager::JOYPAD_BTN::DOWN);
}

bool Player::GetInputAttack(void)
{
	if (stamina_ <= 0) return false;

#ifdef _DEBUG
	if (ImGui::GetIO().WantCaptureMouse)
	{
		return InputManager::GetInstance().IsPadBtnTrgDown(
			InputManager::JOYPAD_NO::PAD1, InputManager::JOYPAD_BTN::R_TRIGGER);
	}
#endif

	auto& ins = InputManager::GetInstance();
	return ins.IsClickMouseLeft() || ins.IsPadBtnTrgDown(
		InputManager::JOYPAD_NO::PAD1, InputManager::JOYPAD_BTN::R_TRIGGER);
}

bool Player::GetInputEvasion(void)
{
	if (stamina_ <= 0) return false;
	auto& ins = InputManager::GetInstance();
	return ins.IsTrgDown(KEY_INPUT_SPACE) || ins.IsPadBtnNew(
		InputManager::JOYPAD_NO::PAD1, InputManager::JOYPAD_BTN::RIGHT);
}

bool Player::GetInputRecovery(void)
{
	if (recoveryBottleCount_ <= 0) return false;
	auto& ins = InputManager::GetInstance();
	return ins.IsTrgDown(KEY_INPUT_R) || ins.IsPadBtnNew(
		InputManager::JOYPAD_NO::PAD1, InputManager::JOYPAD_BTN::LEFT);
}

Player::STATE Player::GetNextStateByInput(void)
{
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

	for (const auto& entry : INPUT_TABLE)
	{
		if ((this->*entry.isInput)()) return entry.nextState;
	}

	if (!AsoUtility::EqualsVZero(GetInputMoveDir())) return STATE::MOVE;
	return STATE::IDLE;
}

void Player::ProcessVinclible(void)
{
	if (invincibleTimer_ <= 0.0f) return;
	invincibleTimer_ -= scnMng_.GetDeltaTime();
}

void Player::ProcessStaminaRecovery(void)
{
	const auto& status = GetData()["status"];
	float maxSt = status["maxStamina"];
	if (stamina_ >= maxSt)
	{
		stamina_ = maxSt;
		return;
	}

	if (state_ != static_cast<int>(STATE::IDLE) 
		&& state_ != static_cast<int>(STATE::MOVE)
		&& state_ != static_cast<int>(STATE::RECOVERY)) return;
	stamina_ += status["recoveryStSpeed"].get<float>() * scnMng_.GetDeltaTime();
}
