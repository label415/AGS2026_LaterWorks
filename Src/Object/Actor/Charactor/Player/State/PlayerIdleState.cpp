#include "../../../../../Utility/AsoUtility.h"
#include "../Player.h"
#include "PlayerIdleState.h"

PlayerIdleState::PlayerIdleState(void)
{
}

PlayerIdleState::~PlayerIdleState(void)
{
}

void PlayerIdleState::StartState(Player& player)
{
	// アニメーションを再生
	player.animationController_->Play(static_cast<int>(Player::ANIM_TYPE::IDLE));
}

void PlayerIdleState::Update(Player& player)
{
	// 入力に応じて状態遷移
	Player::STATE next = player.GetNextStateByInput();
	if (next != Player::STATE::IDLE)
	{
		player.ChangeState(next);
	}
}

void PlayerIdleState::EndState(Player& player){}
