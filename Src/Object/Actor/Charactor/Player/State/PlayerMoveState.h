#pragma once
#include "../../StateBase.h"

class Player;

class PlayerMoveState : public StateBase<Player>
{
public:

	// コンストラクタ  
	PlayerMoveState(void);

	// 開始
	void StartState(Player& player)override;

	// 更新
	void Update(Player& player)override;

	// 終了
	void EndState(Player& player)override;

	// 歩き時の設定
	void SetWalk(Player& player);

	// 走り時の設定
	void SetRun(Player& player);

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
	void SetMoveType(Player& player);
};

