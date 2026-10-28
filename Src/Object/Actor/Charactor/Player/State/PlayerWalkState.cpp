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

	// •à‚«SE
	int handle = ResourceManager::GetInstance().Load(
		ResourceManager::SRC::SE_PLAYER_WAKE).handleId_;
	SoundManager::GetInstance().PlayLoopSE(
		SoundManager::SE_ID::PLAYER_WAKE, handle, Player::SE_VOLUME_WALK);
}

void PlayerWalkState::Update(Player& player)
{
}

void PlayerWalkState::EndState(Player& player)
{
	SoundManager::GetInstance().StopSE(SoundManager::SE_ID::PLAYER_WAKE);
}
