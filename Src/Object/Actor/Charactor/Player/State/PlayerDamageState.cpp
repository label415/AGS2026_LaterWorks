#include "../../../../../Utility/AsoUtility.h"
#include "../../../../../Manager/SoundManager.h"
#include "../../../../../Manager/ResourceManager.h"
#include "../../../../../Manager/InputManager.h"
#include "../../../../../Manager/SceneManager.h"
#include "../../../UI/UISt.h"
#include "../Player.h"
#include "PlayerDamageState.h"

PlayerDamageState::PlayerDamageState(void)
{
}

PlayerDamageState::~PlayerDamageState(void)
{
}

void PlayerDamageState::StartState(Player& player)
{
	// 無敵状態にする
	player.invincibleTimer_ = Player::INVINCIBLE_TIME;

	// ダメージエフェクト再生
	player.effType_ = Player::EFFECT::BLOOD;
	player.effect_->Play(static_cast<int>(player.effType_));
	player.effect_->SetEffectScl(static_cast<int>(player.effType_), Player::EFFECT_BLOOD_SCALE);
	player.effect_->SetEffectPos(static_cast<int>(Player::EFFECT::BLOOD),
		MV1GetFramePosition(player.transform_.modelId, Player::HIT_EFFECT_FRAME_NO));

	// ダメージアニメーション再生
	player.animationController_->Play(static_cast<int>(Player::ANIM_TYPE::DOWN), false);

	// ダメージSE再生
	player.sngMng_.PlaySoundId(SoundManager::SOUND_ID::PLAYER_DMAGE);
}

void PlayerDamageState::Update(Player& player)
{
	if (!player.animationController_->IsEnd())return;

	if(player.animationController_->GetPlayType() 
		== static_cast<int>(Player::ANIM_TYPE::DOWN))
	{
		player.animationController_->Play(
			static_cast<int>(Player::ANIM_TYPE::UP), false);
		player.invincibleTimer_ = player.INVINCIBLE_TIME;
	}
	else {
		player.ChangeState(Player::STATE::IDLE);
	}
}

void PlayerDamageState::EndState(Player& player)
{
}
