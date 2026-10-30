#include "../../../../../Utility/AsoUtility.h"
#include "../../../../../Manager/SoundManager.h"
#include "../../../../../Manager/ResourceManager.h"
#include "../Player.h"
#include "PlayerWalkState.h"

PlayerWalkState::PlayerWalkState(void)
{
}

PlayerWalkState::~PlayerWalkState(void)
{
}

void PlayerWalkState::StartState(Player& player)
{
	// ˆÚ“®—Ê‚Ì‰Šú‰»
	player.moveSpeed_ = 0.0f;
	player.movePow_ = AsoUtility::VECTOR_ZERO;

	player.animationController_->Play(static_cast<int>(Player::ANIM_TYPE::RUN));

	// •à‚«SE“o˜^
	int walkSe = ResourceManager::GetInstance().Load(
		ResourceManager::SRC::SE_PLAYER_WAKE).handleId_;

	// •à‚«SEÄ¶
	player.sngMng_.PlaySoundId(SoundManager::SOUND_ID::PLAYER_WAKE, true);
}

void PlayerWalkState::Update(Player& player)
{
}

void PlayerWalkState::EndState(Player& player)
{
	// •à‚«SE’â~
	player.sngMng_.StopSound(SoundManager::SOUND_ID::PLAYER_WAKE);
}
