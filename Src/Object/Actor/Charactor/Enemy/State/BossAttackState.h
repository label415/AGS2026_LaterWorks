#pragma once
#include "../../StateBase.h"

class EnemyBoss;

class BossAttackState : public StateBase<EnemyBoss>
{
public:

	// コンストラクタ
	BossAttackState(int animNo);

	// 開始
	void StartState(EnemyBoss& boss)override;

	// 更新
	void Update(EnemyBoss& boss)override;

	// 終了
	void EndState(EnemyBoss& boss)override;

private:

	// アニメーション番号
	int animNo_;

};

