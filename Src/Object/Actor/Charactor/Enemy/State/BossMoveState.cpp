#include "../../../../../Utility/AsoUtility.h"
#include "../EnemyBoss.h"
#include "BossMoveState.h"

BossMoveState::BossMoveState(void)
{
}

void BossMoveState::StartState(EnemyBoss& enemy)
{
	SetWalk(enemy);
}

void BossMoveState::Update(EnemyBoss& enemy)
{
	enemy.moveDir_ = AsoUtility::VNormali(*enemy.targetPos_, enemy.transform_.pos);

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

void BossMoveState::SetWalk(EnemyBoss& enemy)
{
	// 移動速度を設定
	enemy.moveSpeed_ = enemy.GetData()["movement"]["speedMove"].get<float>();

	// アニメーションを再生
	enemy.animationController_->Play(static_cast<int>(EnemyBoss::ANIM_TYPE::WALK));

	// 走りSEを止めてから歩きSE再生
}

void BossMoveState::SetRun(EnemyBoss& enemy)
{
	// 移動速度を設定
	enemy.moveSpeed_ = enemy.GetData()["movement"]["speedDash"].get<float>();

	// アニメーションを再生
	enemy.animationController_->Play(static_cast<int>(EnemyBoss::ANIM_TYPE::RUN));

	// 歩きSEを止めてから走りSE再生
}

void BossMoveState::SetMoveType(EnemyBoss& enemy)
{
	// すでに同じ移動タイプの場合は処理しない
	if (moveType_ == nextMoveType_) return;

	// 移動タイプを更新
	moveType_ = nextMoveType_;
	nextMoveType_ = MOVE_TYPE::NONE;

	// 移動タイプに応じた処理を実行
	if (moveType_ == MOVE_TYPE::RUN)
	{
		SetRun(enemy);
	}
	else
	{
		SetWalk(enemy);
	}
}
