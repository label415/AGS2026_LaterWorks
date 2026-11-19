#include "../EnemyBoss.h"
#include "BossIdleState.h"

BossIdleState::BossIdleState(void)
{
}

void BossIdleState::StartState(EnemyBoss& boss)
{
	const int animType = static_cast<int>(EnemyBoss::ANIM_TYPE::IDLE);
	boss.animationController_->Play(animType, true, false, boss.animationController_->GetBlendTime(animType));
}

void BossIdleState::Update(EnemyBoss& boss)
{
}

void BossIdleState::EndState(EnemyBoss& boss)
{
}
