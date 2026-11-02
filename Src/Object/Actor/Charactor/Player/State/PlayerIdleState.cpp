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
	if (player.GetInputAttack()) {
		player.ChangeState(Player::STATE::ATTACK);
		return;
	}
	if(player.GetInputEvasion()){
		player.ChangeState(Player::STATE::EVASION);
		return;
	}
	if(player.GetInputRecovery()){
		player.ChangeState(Player::STATE::RECOVERY);
		return;
	}
	if(!AsoUtility::Equals(
		player.GetInputMoveDir(),AsoUtility::VECTOR_ZERO))
	{
		player.ChangeState(Player::STATE::MOVE);
		return;
	}
}

void PlayerIdleState::EndState(Player& player)
{
}
