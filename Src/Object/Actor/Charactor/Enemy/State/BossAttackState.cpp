#include "../../../../../Utility/AsoUtility.h"
#include "../EnemyBoss.h"
#include "BossAttackState.h"

BossAttackState::BossAttackState(int animNo)
	: animNo_(animNo)
{
}

void BossAttackState::StartState(EnemyBoss& boss)
{
	// アニメーションを再生
	boss.animationController_->Play(animNo_);
}

void BossAttackState::Update(EnemyBoss& boss)
{
}

void BossAttackState::EndState(EnemyBoss& boss)
{
}
