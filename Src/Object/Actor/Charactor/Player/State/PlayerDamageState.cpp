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

void PlayerDamageState::StartState(Player& player)
{
	// 無敵状態にする
	player.invincibleTimer_ = player.GetData()["status"]["invincibleTime"].get<float>();

	// ダメージエフェクト再生
	player.effType_ = Player::EFFECT::BLOOD;
	player.effect_->Play(static_cast<int>(player.effType_));
	player.effect_->SetEffectScl(static_cast<int>(player.effType_), player.GetData()["effectAndSound"]["effectBloodScale"].get<VECTOR>());
	player.effect_->SetEffectPos(static_cast<int>(Player::EFFECT::BLOOD),
		MV1GetFramePosition(player.transform_.modelId, player.GetData()["effectAndSound"]["hitEffectFrameNo"].get<int>()));

	// ダメージアニメーション再生
	const int animType = static_cast<int>(Player::ANIM_TYPE::DOWN);
	player.animationController_->Play(animType, false, false, player.animationController_->GetBlendTime(animType));

	// ダメージSE再生
	player.sngMng_.PlaySoundId(SoundManager::SOUND_ID::PLAYER_DMAGE);
}

void PlayerDamageState::Update(Player& player)
{
	if (!player.animationController_->IsEnd())return;

	if(player.animationController_->GetPlayType() 
		== static_cast<int>(Player::ANIM_TYPE::DOWN))
	{
		const int upType = static_cast<int>(Player::ANIM_TYPE::UP);
		player.animationController_->Play(
			upType, false, false, player.animationController_->GetBlendTime(upType));
		player.invincibleTimer_ = player.GetData()["status"]["invincibleTime"].get<float>();
	}
	else {
		player.ChangeState(static_cast<int>(Player::STATE::IDLE));
	}
}

void PlayerDamageState::EndState(Player& player)
{
}
