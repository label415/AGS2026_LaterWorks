#pragma once

class EnemyBase;

class EnemyStateBase
{
public:

	// コンストラクタ
	EnemyStateBase(void);

	// デストラクタ
	virtual ~EnemyStateBase(void) = 0;

	// 開始
	virtual void StartState(EnemyBase& enemyData) = 0;

	// 更新
	virtual void Update(EnemyBase& enemyData) = 0;

	// 終了
	virtual void EndState(EnemyBase& enemyData) = 0;
};

