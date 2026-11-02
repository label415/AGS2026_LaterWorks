#include "../../../../../Utility/AsoUtility.h"
#include "../../../../../Manager/SoundManager.h"
#include "../../../../../Manager/ResourceManager.h"
#include "../../../../../Manager/InputManager.h"
#include "../../../../../Manager/SceneManager.h"
#include "../../../Weapon/WeaponSaw.h"
#include "../../../UI/UISt.h"
#include "../Player.h"
#include "PlayerAttackState.h"

PlayerAttackState::PlayerAttackState(void)
{
}

PlayerAttackState::~PlayerAttackState(void)
{
}

void PlayerAttackState::StartState(Player& player)
{
	player.stateAtkCombo_ = Player::STATE_ATTACK_COMBO::COMBO_1;

	// データの参照と初期設定
	auto& comboData = player.atkComboData_.at(player.stateAtkCombo_);
	comboData.isNextCombo = false;

	// 追加の初期処理があれば実行
	if (comboData.extraInit) {
		comboData.extraInit();
	}

	// 攻撃アニメーション再生
	player.animationController_->Play(static_cast<int>(comboData.animType), false);
}

void PlayerAttackState::Update(Player& player)
{
	//// 通常状態からの新規攻撃開始
	//if (player.GetInputAttack())
	//{
	//}

	//// 現在のコンボデータを取得
	//auto& comboData = atkComboData_.at(stateAtkCombo_);
	//float currentStep = animationController_->GetPlayAnim().step;

	//if (comboData.moveSpeed > 0.0f && currentStep < comboData.stepCollisionStart)
	//{
	//	// 入力方向を取得
	//	VECTOR inputDir = GetInputDirection();

	//	// 入力方向がゼロベクトルでない場合のみ処理
	//	if (AsoUtility::EqualsVZero(inputDir))return;

	//	// 入力方向へ回転
	//	float targetAngleY = atan2f(inputDir.x, inputDir.z);
	//	Quaternion targetRot = Quaternion::AngleAxis(targetAngleY, AsoUtility::AXIS_Y);
	//	transform_.quaRot = targetRot;

	//	// 向いた正面方向へ向かって踏み込み移動
	//	VECTOR forward = transform_.GetForward();
	//	VECTOR targetPos = VAdd(transform_.pos, VScale(forward, comboData.moveSpeed));

	//	transform_.pos = AsoUtility::Lerp(transform_.pos, targetPos, ATTACK_STEP_LERP_RATE);
	//}

	//// 追加更新処理
	//if (comboData.extraUpdate) {
	//	comboData.extraUpdate();
	//}

	//// コンボ入力受付判定
	//if (comboData.IsValidCombo(currentStep)) {
	//	// 通常の攻撃入力、または固有の条件（actionNextCombo）を満たした場合
	//	bool isCustomAction = comboData.actionNextCombo ? comboData.actionNextCombo() : false;
	//	if (isAttackInput || isCustomAction) {
	//		comboData.isNextCombo = true;
	//	}
	//}

	//// 衝突判定の処理
	//if (comboData.IsValidCollsion(currentStep)) {
	//	for (const auto& weapon : weapons_) {
	//		weapon->SetCollider();
	//	}

	//}
	//else {
	//	for (const auto& weapon : weapons_) {
	//		weapon->ClearCollider();
	//	}
	//}

	//// SE再生処理
	//if (currentStep == ATTACK_SE_TRIGGER_STEP) {
	//	int bgm_ = resMng_.Load(ResourceManager::SRC::SE_PLAYER_WEAPON_1).handleId_;
	//	SoundManager::GetInstance().PlaySE(
	//		SoundManager::SE_ID::PLAYER_WEPON_SE1, bgm_, SE_VOLUME_ATTACK);
	//}

	//// コンボ遷移チェック
	//bool canInterrupt = comboData.IsValidInterrupt(currentStep);
	//bool isAnimEnd = animationController_->IsEnd();

	//// 追加割込条件のチェック
	//if (comboData.isExtraInterrupt && comboData.isExtraInterrupt(comboData)) {
	//	canInterrupt = true;
	//}

	//// 次のコンボへ繋ぐ場合、またはアニメーションが終了した場合に処理
	//if ((canInterrupt && comboData.isNextCombo) || isAnimEnd) {
	//	// 次のコンボへ繋ぐ場合
	//	if (comboData.isNextCombo && comboData.nextCombo != STATE_ATTACK_COMBO::MAX) {
	//		// 次のコンボへ状態遷移
	//		stateAtkCombo_ = comboData.nextCombo;

	//		for (const auto& weapon : weapons_) {
	//			weapon->ClearCollider();
	//		}

	//		// 新しいコンボデータのセットアップ
	//		auto& nextData = atkComboData_.at(stateAtkCombo_);
	//		nextData.isNextCombo = false;

	//		uiSt_->SetSt(CONSUMPTION_ST_ATTACK);

	//		if (nextData.extraInit) {
	//			nextData.extraInit();
	//		}

	//		animationController_->Play(static_cast<int>(nextData.animType), false);
	//	}
	//	// アニメーションが終了した場合、または追加終了条件を満たした場合
	//	else if (isAnimEnd || (comboData.isExtraEnd && comboData.isExtraEnd()))
	//	{
	//		state_ = STATE::IDLE;
	//		stateAtkCombo_ = STATE_ATTACK_COMBO::COMBO_1;
	//		comboData.isNextCombo = false;

	//		for (const auto& weapon : weapons_) {
	//			weapon->ClearCollider();
	//		}
	//	}
	//}
}

void PlayerAttackState::EndState(Player& player)
{
}
