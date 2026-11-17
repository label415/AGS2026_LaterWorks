#include "BossMoveState.h"

BossMoveState::BossMoveState(void)
{
}

BossMoveState::~BossMoveState(void)
{
}

void BossMoveState::StartState(EnemyBase& enemy)
{
}

void BossMoveState::Update(EnemyBase& enemy)
{
	// ˆÚ“®—Ê‚ÌXV
	enemy.movePow_ =
		VAdd(enemy.movePow_, VScale(enemy.moveDir_, enemy.moveSpeed_));

}

void BossMoveState::EndState(EnemyBase& enemy)
{
}
