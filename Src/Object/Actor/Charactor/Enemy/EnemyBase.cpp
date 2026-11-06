#include "../../../../Utility/AsoUtility.h"
#include "EnemyBase.h"

EnemyBase::EnemyBase(void)
	: CharactorBase()
{
}

EnemyBase::~EnemyBase(void){}

void EnemyBase::Draw(void)
{
	CharactorBase::Draw();
}

void EnemyBase::ChangeState(int state)
{
}

