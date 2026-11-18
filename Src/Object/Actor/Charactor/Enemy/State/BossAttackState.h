#pragma once
#include "../../StateBase.h"

class EnemyBoss;

class BossAttackState : public StateBase<EnemyBoss>
{
public:

	// コンストラクタ
	BossAttackState(void);

	// 開始
	void StartState(EnemyBoss& enemyData)override;

	// 更新
	void Update(EnemyBoss& enemyData)override;

	// 終了
	void EndState(EnemyBoss& enemyData)override;

};

