#pragma once
#include "EnemyStateBase.h"

class BossChaseState : public EnemyStateBase
{
public:

	// コンストラクタ
	BossChaseState(void);

	// デストラクタ
	~BossChaseState(void)override;

	// 開始
	void StartState(EnemyBase& enemyData)override;

	// 更新
	virtual void Update(EnemyBase& enemyData)override;

	// 終了
	virtual void EndState(EnemyBase& enemyData)override;

};