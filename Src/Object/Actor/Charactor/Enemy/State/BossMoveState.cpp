#include "../../../../../Utility/AsoUtility.h"
#include "../EnemyBoss.h"
#include "BossMoveState.h"

BossMoveState::BossMoveState(float moveSpeed, int animNo)
	: moveSpeed_(moveSpeed)
	, animNo_(animNo)
{
}

void BossMoveState::StartState(EnemyBoss& enemy)
{
	// 移動速度を設定
	enemy.moveSpeed_ = moveSpeed_;

	// アニメーションを再生
	enemy.animationController_->Play(animNo_);

}

void BossMoveState::Update(EnemyBoss& enemy)
{
	// 追尾方向を取得
	enemy.SetTargetDir();

	// 移動量の更新
	enemy.movePow_ =
		VAdd(enemy.movePow_, VScale(enemy.moveDir_, enemy.moveSpeed_));

	// 移動量が移動速度を超えないように制限
	if (VSize(enemy.movePow_) > enemy.moveSpeed_)
	{
		// 移動量を正規化して移動速度にスケーリング
		enemy.movePow_ = VScale(VNorm(enemy.movePow_), enemy.moveSpeed_);
	}
}

void BossMoveState::EndState(EnemyBoss& enemy)
{
}
