#include "../../../../../Utility/AsoUtility.h"
#include "../../../../../Manager/SoundManager.h"
#include "../../../../../Manager/ResourceManager.h"
#include "../../../../../Manager/InputManager.h"
#include "../../../../../Manager/SceneManager.h"
#include "../../../UI/UISt.h"
#include "../Player.h"
#include "PlayerRecoveryState.h"

PlayerRecoveryState::PlayerRecoveryState(void)
{
}

PlayerRecoveryState::~PlayerRecoveryState(void)
{
}

void PlayerRecoveryState::StartState(Player& player)
{
	// 回復アニメーション再生
	player.animationController_->Play(static_cast<int>(Player::ANIM_TYPE::RECOVERY), false);

	// 回復エフェクト再生
	VECTOR effectPos =
		MV1GetFramePosition(player.transform_.modelId, Player::HIT_EFFECT_FRAME_NO);
	VECTOR effectScale =
		VScale(AsoUtility::VECTOR_ONE, Player::EFFECT_RECOVERY_SCALE);
	VECTOR effectRot = AsoUtility::VECTOR_ZERO;
	player.effect_->Play(
		static_cast<int>(Player::EFFECT::HP_ABSOLUTE),
		effectPos, effectRot, effectScale);

	// 回復SE再生
	player.sngMng_.PlaySoundId(SoundManager::SOUND_ID::PLAYER_HER);
}

void PlayerRecoveryState::Update(Player& player)
{
	if(player.animationController_->IsEnd())
	{
		player.ChangeState(Player::STATE::IDLE);
	}
}

void PlayerRecoveryState::EndState(Player& player)
{
}
