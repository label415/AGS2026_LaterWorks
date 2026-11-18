#pragma once
#include "../../StateBase.h"

class EnemyBoss;

class BossMoveState : public StateBase<EnemyBoss>
{
public:

	// コンストラクタ
	BossMoveState(void);

	// 開始
	void StartState(EnemyBoss& enemy)override;

	// 更新
	void Update(EnemyBoss& enemy)override;

	// 終了
	void EndState(EnemyBoss& enemy)override;

	// 歩き時の設定
	void SetWalk(EnemyBoss& enemy);

	// 走り時の設定
	void SetRun(EnemyBoss& enemy);

private:

	// 移動タイプ
	enum class MOVE_TYPE
	{
		NONE,
		WALK,
		RUN,
	};

	// 現在の移動タイプ
	MOVE_TYPE moveType_;

	MOVE_TYPE nextMoveType_;

	// 移動処理の変更
	void SetMoveType(EnemyBoss& enemy);
};

