#pragma once
#include "EnemyStateBase.h"

class BossMoveState : public EnemyStateBase
{
public:

	// コンストラクタ
	BossMoveState(void);

	// デストラクタ
	~BossMoveState(void)override;

	// 開始
	void StartState(EnemyBase& enemy)override;

	// 更新
	virtual void Update(EnemyBase& enemy)override;

	// 終了
	virtual void EndState(EnemyBase& enemy)override;

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

};

