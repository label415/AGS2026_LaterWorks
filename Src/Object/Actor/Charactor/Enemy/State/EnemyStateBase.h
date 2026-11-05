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
	virtual void StartState(EnemyBase& player) = 0;

	// 更新
	virtual void Update(EnemyBase& player) = 0;

	// 終了
	virtual void EndState(EnemyBase& player) = 0;
};

