#pragma once
#include "../../StateBase.h"

class EnemyBoss;

class BossIdleState : public StateBase<EnemyBoss>
{
public:

	// コンストラクタ
	BossIdleState(void);

	// 開始
	void StartState(EnemyBoss& boss)override;

	// 更新
	void Update(EnemyBoss& boss)override;

	// 終了
	void EndState(EnemyBoss& boss)override;

};

