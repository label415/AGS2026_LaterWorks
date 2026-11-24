#include "../EnemyBoss.h"
#include "EnemyStateManager.h"

EnemyStateManager::EnemyStateManager(EnemyBoss& enemyBoss)
	: currentState_(nullptr)
	, state_(-1)
	, enemyBoss_(enemyBoss)
{
	states_.clear();
}

EnemyStateManager::~EnemyStateManager(void)
{
}

void EnemyStateManager::AddState(int stateId, std::unique_ptr<StateBase<EnemyBoss>> state)
{
	states_.emplace(stateId, std::move(state));
}

void EnemyStateManager::AddConditions(void)
{
}

void EnemyStateManager::Update(void)
{
	// 開始処理
	currentState_->Update(enemyBoss_);
}

void EnemyStateManager::Release(void)
{
	currentState_->EndState(enemyBoss_);
	currentState_ = nullptr;
	states_.clear();
}

void EnemyStateManager::ChangerState(int state)
{
	// 同じステートの時は処理しない
	if (state_ == state)return;

	// 終了時の処理
	if(currentState_ != nullptr)
	{
		currentState_->EndState(enemyBoss_);
	}

	// ステートを変更
	state_ = state;
	currentState_ = states_.at(state_).get();

	// 開始処理
	currentState_->StartState(enemyBoss_);
}
