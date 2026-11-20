#include "../../../../../Utility/AsoUtility.h"
#include "../../../../../Manager/SoundManager.h"
#include "../../../../../Manager/ResourceManager.h"
#include "../../../../../Manager/InputManager.h"
#include "../../../../../Manager/SceneManager.h"
#include "../../../Weapon/WeaponManager.h"
#include "../../../UI/UISt.h"
#include "../Player.h"
#include "PlayerAttackState.h"

PlayerAttackState::PlayerAttackState(void)
{
}

void PlayerAttackState::StartState(Player& player)
{
	player.stateAtkCombo_ = Player::STATE_ATTACK_COMBO::COMBO_1;

	// 攻撃アニメーション再生
	auto& comboData = player.atkComboData_.at(player.stateAtkCombo_);
	player.animationController_->Play(static_cast<int>(comboData.animType), false);
	player.stamina_ -= player.GetData()["status"]["consumptionStAttack"].get<float>();
}

void PlayerAttackState::Update(Player& player)
{
	auto& comboData = player.atkComboData_.at(player.stateAtkCombo_);
	float currentStep = player.animationController_->GetPlayAnim().step;
	const auto& data = player.GetData();

	if (comboData.moveSpeed > 0.0f && currentStep < comboData.stepCollisionStart)
	{
		VECTOR inputDir = player.GetInputMoveDir();
		if (AsoUtility::EqualsVZero(inputDir)) return;

		float targetAngleY = atan2f(inputDir.x, inputDir.z);
		player.transform_.quaRot = Quaternion::AngleAxis(targetAngleY, AsoUtility::AXIS_Y);

		VECTOR forward = player.transform_.GetForward();
		VECTOR targetPos = VAdd(player.transform_.pos, VScale(forward, comboData.moveSpeed));
		player.transform_.pos = AsoUtility::Lerp(
			player.transform_.pos, targetPos, data["movement"]["attackStepLerpRate"].get<float>());
	}

	if (comboData.IsValidCombo(currentStep) && player.GetInputAttack())
	{
		comboData.isNextCombo = true;
	}

	if (comboData.IsValidCollsion(currentStep))
	{
		player.weapons_->SetCollider();
	}
	else
	{
		player.weapons_->ClearCollider();
	}

	if (currentStep == data["effectAndSound"]["attackSeTriggerStep"].get<float>())
	{
		player.sngMng_.PlaySoundId(SoundManager::SOUND_ID::PLAYER_WEPON_SE1);
	}

	bool canInterrupt = comboData.IsValidInterrupt(currentStep);
	bool isAnimEnd = player.animationController_->IsEnd();

	if ((canInterrupt && comboData.isNextCombo) || isAnimEnd)
	{
		if (comboData.isNextCombo && comboData.nextCombo != Player::STATE_ATTACK_COMBO::MAX)
		{
			player.stateAtkCombo_ = comboData.nextCombo;
			player.weapons_->ClearCollider();

			auto& nextData = player.atkComboData_.at(player.stateAtkCombo_);
			nextData.isNextCombo = false;

			player.animationController_->Play(static_cast<int>(nextData.animType), false);
			player.stamina_ -= data["status"]["consumptionStAttack"].get<float>();
		}
		else if (isAnimEnd)
		{
			player.ChangeState(static_cast<int>(Player::STATE::IDLE));
			player.stateAtkCombo_ = Player::STATE_ATTACK_COMBO::COMBO_1;
			comboData.isNextCombo = false;
			player.weapons_->ClearCollider();
		}
	}
}

void PlayerAttackState::EndState(Player& player)
{
}
