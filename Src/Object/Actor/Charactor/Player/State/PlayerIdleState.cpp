#include "../../../../../Utility/AsoUtility.h"
#include "../Player.h"
#include "PlayerIdleState.h"

PlayerIdleState::PlayerIdleState(void)
{
}

void PlayerIdleState::StartState(Player& player)
{
	// アニメーションを再生
	const int animType = static_cast<int>(Player::ANIM_TYPE::IDLE);
	player.animationController_->Play(animType, player.animationController_->GetBlendTime(animType));
}

void PlayerIdleState::Update(Player& player)
{
	// 入力に応じて状態遷移
	Player::STATE next = player.GetNextStateByInput();
	if (next != Player::STATE::IDLE)
	{
		player.ChangeState(static_cast<int>(next));
	}
}

void PlayerIdleState::EndState(Player& player){}
