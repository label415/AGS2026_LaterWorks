#include <fstream>
#include <DxLib.h>
#include "../../../../Manager/ResourceManager.h"
#include "../../../../Manager/SceneManager.h"
#include "../../../../Manager/CollisionManager.h"
#include "../../../../Manager/Camera.h"
#include "../../../../Manager/InputManager.h"
#include "../../../../Manager/SoundManager.h"
#include "../../../../Utility/AsoUtility.h"
#include "../../../../Utility/ColliderUtility.h"
#include "../../../../Utility/MatrixUtility.h"
#include "../../../../Application.h"
#include "../../../../Common/Quaternion.h"
#include "../../../Common/Collider/ColliderLine.h"
#include "../../../Common/Collider/ColliderCapsule.h"
#include "../../../Common/Collider/ColliderSphere.h"
#include "../../../Common/Collider/ColliderModel.h"
#include "../Libs/nlohmann/json.hpp"
#include "../../Weapon/WeaponSaw.h"
#include "../../UI/UISt.h"
#include "../../UI/UIHp.h"
#include "../../UI/UIRecovery.h"
#include "State/PlayerStateBase.h"
#include "State/PlayerMoveState.h"
#include "State/PlayerIdleState.h"
#include "State/PlayerAttackState.h"
#include "State/PlayerEvasionState.h"
#include "State/PlayerDamageState.h"
#include "State/PlayerRecoveryState.h"
#include "Player.h"

Player::Player(void)
	: animLockPos_(AsoUtility::VECTOR_ZERO)
	, effType_(EFFECT::NONE)
	, invincibleTimer_(0.0f)
	, isComboNext_(false)
	, stateAtkCombo_(STATE_ATTACK_COMBO::COMBO_1)
	, state_(STATE::IDLE)
	, isChangeState_(false)
	, stamina_(MAX_STAMINA)
	, stCoolTime_(0.0f)
	, currentState_(nullptr)
	, nextState_(STATE::IDLE)
	, hp_(MAX_HP)
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
	// 基底クラスの解放処理
	CharactorBase::Release();
}

void Player::InitLoad(void)
{
	//プレイヤーモデル
	transform_.SetModel(resMng_.LoadModelDuplicate(
		ResourceManager::SRC::MODEL_PLAYER));

	// 武器初期化
	InitWeapon();

	// サウンド初期化
	InitSound();
}

void Player::InitTransform(void)
{
	// Transformの初期化
	const auto& playerData =
		resMng_.Load(ResourceManager::SRC::JSON_PLAYER).jsonData_;
	const auto& playerTrans = playerData["transform"];

	// 座標の初期化
	const auto& playerPos = playerTrans["position"];
	transform_.pos = { playerPos["x"], playerPos["y"], playerPos["z"] };

	// 大きさの初期化
	const auto& playerScale = playerTrans["scale"];
	transform_.scl = { playerScale["x"], playerScale["y"], playerScale["z"] };

	// 回転の初期化
	transform_.quaRot = Quaternion::Identity();
	transform_.quaRotLocal = Quaternion::Identity();
	const auto& playerRotY = playerTrans["angleY"];
	transform_.quaRotLocal =
		Quaternion::Mult(transform_.quaRotLocal,
			Quaternion::AngleAxis(AsoUtility::Deg2RadF(playerRotY), AsoUtility::AXIS_Y));

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
		COL_LINE_START_LOCAL_POS,
		COL_LINE_END_LOCAL_POS);

	// マネージャーへ登録
	ownColliders_[static_cast<int>(ColliderBase::SHAPE::LINE)].push_back(colLine);
	colMng_.AddCollider(colLine);

	// カプセルコライダ
	std::vector<ColliderBase::TAG> CapsuleTags = {
	ColliderBase::TAG::ENEMY,
	ColliderBase::TAG::ENEMY_WEPON,
	ColliderBase::TAG::STAGE
	};

	auto colCapsule = std::make_shared<ColliderCapsule>(
		ColliderBase::TAG::PLAYER,
		CapsuleTags,
		&transform_,
		COL_CAPSULE_TOP_LOCAL_POS,
		COL_CAPSULE_DOWN_LOCAL_POS,
		COL_CAPSULE_RADIUS);

	// マネージャーへ登録
	ownColliders_[static_cast<int>(ColliderBase::SHAPE::CAPSULE)].push_back(colCapsule);
	colMng_.AddCollider(colCapsule);
}

void Player::InitAnimation(void)
{
	// アニメーション
	animationController_ = std::make_unique<AnimationController>(transform_.modelId);

	// アニメーション速度の読み込み
	const auto& playerData =
		resMng_.Load(ResourceManager::SRC::JSON_PLAYER).jsonData_;
	const auto& anim = playerData["animationSpeed"];

	// アニメーションの登録
	animationController_->Add(static_cast<int>(ANIM_TYPE::IDLE),
		anim["idle"].get<float>(), resMng_.LoadModelDuplicate(ResourceManager::SRC::ANIM_PLAYER_IDLE));
	animationController_->Add(static_cast<int>(ANIM_TYPE::RUN),
		anim["run"].get<float>(), resMng_.LoadModelDuplicate(ResourceManager::SRC::ANIM_PLAYER_RUN));
	animationController_->Add(static_cast<int>(ANIM_TYPE::FAST_RUN),
		anim["fastRun"].get<float>(), resMng_.LoadModelDuplicate(ResourceManager::SRC::ANIM_PLAYER_RUN));
	animationController_->Add(static_cast<int>(ANIM_TYPE::ATTACK_1),
		anim["attack1"].get<float>(), resMng_.LoadModelDuplicate(ResourceManager::SRC::ANIM_PLSYER_ATTACK_1));
	animationController_->Add(static_cast<int>(ANIM_TYPE::ATTACK_2),
		anim["attack2"].get<float>(), resMng_.LoadModelDuplicate(ResourceManager::SRC::ANIM_PLSYER_ATTACK_2));
	animationController_->Add(static_cast<int>(ANIM_TYPE::ATTACK_3),
		anim["attack3"].get<float>(), resMng_.LoadModelDuplicate(ResourceManager::SRC::ANIM_PLSYER_ATTACK_3));
	animationController_->Add(static_cast<int>(ANIM_TYPE::EVASION),
		anim["evasion"].get<float>(), resMng_.LoadModelDuplicate(ResourceManager::SRC::ANIM_PLAYER_EVASION));
	animationController_->Add(static_cast<int>(ANIM_TYPE::DOWN),
		anim["down"].get<float>(), resMng_.LoadModelDuplicate(ResourceManager::SRC::ANIM_PLAYER_DOWN));
	animationController_->Add(static_cast<int>(ANIM_TYPE::UP),
		anim["up"].get<float>(), resMng_.LoadModelDuplicate(ResourceManager::SRC::ANIM_PLAYER_UP));
	animationController_->Add(static_cast<int>(ANIM_TYPE::RECOVERY),
		anim["recovery"].get<float>(), resMng_.LoadModelDuplicate(ResourceManager::SRC::ANIM_PLAYER_RECOVERY));

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

	// 攻撃コンボデータの読み込み
	const auto& playerData =
		resMng_.Load(ResourceManager::SRC::JSON_PLAYER).jsonData_;
	const auto& playerCombo = playerData.at("combo");

	// 攻撃コンボデータの登録
	ATTACK_COMBO data;
	int cont = 0;
	for (const auto& combo : playerCombo)
	{
		data = {
			static_cast<ANIM_TYPE>(combo.at("animType").get<int>()),
			combo.at("inputStart").get<float>(),
			combo.at("inputEnd").get<float>(),
			combo.at("colStart").get<float>(),
			combo.at("colEnd").get<float>(),
			combo.at("stepInterrupt").get<float>(),
			combo.at("moveSpeed").get<float>(),
			static_cast<STATE_ATTACK_COMBO>(combo.at("nextCombo").get<int>()),
		};

		atkComboData_.emplace(static_cast<STATE_ATTACK_COMBO>(cont), data);

		cont++;
	}

	// 攻撃コンボ状態の初期化
	stateAtkCombo_ = STATE_ATTACK_COMBO::COMBO_1;

	// 攻撃コンボの次の攻撃を受け付けるかどうかのフラグを初期化
	isComboNext_ = false;

	// 各状態格納
	InitState();
}

void Player::InitWeapon(void)
{
	// 武器
	auto weaponSaw = std::make_unique<WeaponSaw>(transform_, WEAPON_ATTACH_FRAME_NO);
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
	seHandlId = resMng_.Load(ResourceManager::SRC::SE_PLAYER_WAKE).handleId_;
	sngMng_.Add(SoundManager::SOUND_ID::PLAYER_WAKE, seHandlId, SE_VOLUME);
	// 走りSE
	seHandlId = resMng_.Load(ResourceManager::SRC::SE_PLAYER_RUN).handleId_;
	sngMng_.Add(SoundManager::SOUND_ID::PLAYER_RAN, seHandlId, SE_VOLUME);
	// 回復SE
	seHandlId = resMng_.Load(ResourceManager::SRC::SE_PLAYER_RECOVERY).handleId_;
	sngMng_.Add(SoundManager::SOUND_ID::PLAYER_RAN, seHandlId, SE_VOLUME);
	// 回避SE
	seHandlId = resMng_.Load(ResourceManager::SRC::SE_PLAYER_EVASION).handleId_;
	sngMng_.Add(SoundManager::SOUND_ID::PLAYER_RAN, seHandlId, SE_VOLUME);
	// 攻撃SE
	seHandlId = resMng_.Load(ResourceManager::SRC::SE_PLAYER_WEAPON_1).handleId_;
	sngMng_.Add(SoundManager::SOUND_ID::PLAYER_WEPON_SE1, seHandlId, SE_VOLUME);
	// 攻撃SE2
	seHandlId = resMng_.Load(ResourceManager::SRC::SE_PLAYER_DAMAGE).handleId_;
	sngMng_.Add(SoundManager::SOUND_ID::PLAYER_RAN, seHandlId, SE_VOLUME);
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
	// 前フレームの状態遷移処理
	ApplyChangeState();

	// 状態ごとの更新
	currentState_->Update(*this);

	// 無敵時間処理
	ProcessVinclible();

	// スタミナ回復処理
	ProcessStaminaRecovery();

	// 状態遷移処理
	ApplyChangeState();

	// 武器処理
	for (const auto& weapon : weapons_)
	{
		weapon->Update();
	}

	//移動量の減速処理
	movePow_ = AsoUtility::Lerp(
		movePow_, AsoUtility::VECTOR_ZERO, MOVE_POW_DECREASE);

	//アニメーションの移動量を無効
	SetFrameUserLocalPos(LOCK_POS1, LOCK_FRAME_NO);
}

void Player::UpdateProcessPost(void)
{
	if (targetPos_ == nullptr)return;

	// ターゲットに向かって回転
	VECTOR targetDir = GetTargetDir();
	float targetAngleY = atan2f(targetDir.x, targetDir.z);
	Quaternion targetRot =
		Quaternion::AngleAxis(targetAngleY, AsoUtility::AXIS_Y);
	transform_.quaRot =
		Quaternion::Slerp(transform_.quaRot, targetRot, ROT_TARGET_SLERP_RATE);
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

				if (!enemyCapsule->GetIsAction())continue;

				// 複数同時に当たった場合は大きい方のダメージを優先
				damage = DAMAGE_ENEMY_BODY;
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
				damage = DAMAGE_ENEMY_WEAPON;
			}
		}
	}

	if (damage > 0.0f)
	{
		// ダメージを受ける
		ChangeState(STATE::DAMAGE);
	}
}

void Player::ChangeState(STATE state)
{
	nextState_ = state;
	isChangeState_ = true;
}

void Player::ApplyChangeState(void)
{
	if (!isChangeState_) return;
	isChangeState_ = false;

	if (currentState_ != nullptr)
	{
		currentState_->EndState(*this);
	}

	state_ = nextState_;
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
	if (stamina_ <= 0)return false;

	// 攻撃入力判定
	auto& ins = InputManager::GetInstance();
	bool isAttackInput = ins.IsClickMouseLeft() || ins.IsPadBtnTrgDown(
		InputManager::JOYPAD_NO::PAD1, InputManager::JOYPAD_BTN::R_TRIGGER);

	return isAttackInput;
}

bool Player::GetInputEvasion(void)
{
	// スタミナがないときは処理しない
	if (stamina_ <= 0)return false;

	// 回避入力判定
	auto& ins = InputManager::GetInstance();
	bool isEvasion = ins.IsTrgDown(KEY_INPUT_SPACE) || ins.IsPadBtnNew(
		InputManager::JOYPAD_NO::PAD1, InputManager::JOYPAD_BTN::RIGHT);

	return isEvasion;
}

bool Player::GetInputRecovery(void)
{
	// 回復入力判定
	auto& ins = InputManager::GetInstance();
	bool isRecovery = ins.IsTrgDown(KEY_INPUT_R) || ins.IsPadBtnNew(
		InputManager::JOYPAD_NO::PAD1, InputManager::JOYPAD_BTN::LEFT);

	return isRecovery;
}

void Player::ProcessVinclible(void)
{
	// 無敵時間が0以下の場合は処理を行わない
	if (invincibleTimer_ <= 0.0f)return;

	// 無敵時間の更新
	invincibleTimer_ -= scnMng_.GetDeltaTime();
}

void Player::ProcessStaminaRecovery(void)
{
	// スタミナが最大値に達している場合は処理を行わない
	if (stamina_ >= MAX_STAMINA)
	{
		stamina_ = MAX_STAMINA;
		return;
	}

	// スタミナ回復処理
	stamina_ += RECOVERY_ST_SPEED * scnMng_.GetDeltaTime();
}
