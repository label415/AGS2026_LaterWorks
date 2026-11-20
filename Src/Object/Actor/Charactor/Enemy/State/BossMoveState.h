#pragma once
#include "../../StateBase.h"

class EnemyBoss;

class BossMoveState : public StateBase<EnemyBoss>
{
public:

	// コンストラクタ
	BossMoveState(float moveSpeed, int animNo);

	// 開始
	void StartState(EnemyBoss& enemy)override;

	// 更新
	void Update(EnemyBoss& enemy)override;

	// 終了
	void EndState(EnemyBoss& enemy)override;

private:

	// 移動速度
	float moveSpeed_;

	// アニメーション番号
	int animNo_;
};

