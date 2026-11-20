#include "../EnemyBoss.h"
#include "BossIdleState.h"

BossIdleState::BossIdleState(void)
{
}

void BossIdleState::StartState(EnemyBoss& boss)
{
	boss.animationController_->Play(static_cast<int>(EnemyBoss::ANIM_TYPE::IDLE));
}

void BossIdleState::Update(EnemyBoss& boss)
{
}

void BossIdleState::EndState(EnemyBoss& boss)
{
}
