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

	// 攻撃アニメーション再生
	auto& comboData = player.atkComboData_.at(player.stateAtkCombo_);
	player.animationController_->Play(static_cast<int>(comboData.animType), false);
}

void PlayerAttackState::Update(Player& player)
{
	// 現在のコンボデータを取得
	auto& comboData = player.atkComboData_.at(player.stateAtkCombo_);
	float currentStep = player.animationController_->GetPlayAnim().step;

	if (comboData.moveSpeed > 0.0f && currentStep < comboData.stepCollisionStart)
	{
		// 入力方向を取得
		VECTOR inputDir = player.GetInputMoveDir();

		// 入力方向がゼロベクトルでない場合のみ処理
		if (AsoUtility::EqualsVZero(inputDir))return;

		// 入力方向へ回転
		float targetAngleY = atan2f(inputDir.x, inputDir.z);
		Quaternion targetRot = Quaternion::AngleAxis(targetAngleY, AsoUtility::AXIS_Y);
		player.transform_.quaRot = targetRot;

		// 向いた正面方向へ向かって踏み込み移動
		VECTOR forward = player.transform_.GetForward();
		VECTOR targetPos = VAdd(player.transform_.pos, VScale(forward, comboData.moveSpeed));

		player.transform_.pos = AsoUtility::Lerp(player.transform_.pos, targetPos, Player::ATTACK_STEP_LERP_RATE);
	}

	// コンボ入力受付判定
	if (comboData.IsValidCombo(currentStep))
	{
		if (player.GetInputAttack()) {
			comboData.isNextCombo = true;
		}
	}

	// 衝突判定の処理
	if (comboData.IsValidCollsion(currentStep)) {
		for (const auto& weapon : player.weapons_) {
			weapon->SetCollider();
		}

	}
	else {
		for (const auto& weapon : player.weapons_) {
			weapon->ClearCollider();
		}
	}

	// SE再生処理
	if (currentStep == Player::ATTACK_SE_TRIGGER_STEP)
	{
		player.sngMng_.PlaySoundId(SoundManager::SOUND_ID::PLAYER_WEPON_SE1);
	}

	// コンボ遷移チェック
	bool canInterrupt = comboData.IsValidInterrupt(currentStep);
	bool isAnimEnd = player.animationController_->IsEnd();

	// 次のコンボへ繋ぐ場合、またはアニメーションが終了した場合に処理
	if ((canInterrupt && comboData.isNextCombo) || isAnimEnd) {
		// 次のコンボへ繋ぐ場合
		if (comboData.isNextCombo && comboData.nextCombo != Player::STATE_ATTACK_COMBO::MAX) {
			// 次のコンボへ状態遷移
			player.stateAtkCombo_ = comboData.nextCombo;

			for (const auto& weapon : player.weapons_) {
				weapon->ClearCollider();
			}

			// 新しいコンボデータのセットアップ
			auto& nextData = player.atkComboData_.at(player.stateAtkCombo_);
			nextData.isNextCombo = false;

			player.animationController_->Play(static_cast<int>(nextData.animType), false);
		}
		// アニメーションが終了した場合、または追加終了条件を満たした場合
		else if (isAnimEnd)
		{
			player.ChangeState(Player::STATE::IDLE);
			player.stateAtkCombo_ = Player::STATE_ATTACK_COMBO::COMBO_1;
			comboData.isNextCombo = false;

			for (const auto& weapon : player.weapons_) {
				weapon->ClearCollider();
			}
		}
	}
}

void PlayerAttackState::EndState(Player& player)
{
}
