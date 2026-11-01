#pragma once
#include "PlayerStateBase.h"

class PlayerMoveState : public PlayerStateBase
{
public:
	// コンストラクタ  
	PlayerMoveState(void);

	// デストラクタ  
	~PlayerMoveState(void) override;

	// 開始
	void StartState(Player& player)override;

	// 更新
	void Update(Player& player)override;

	// 終了
	void EndState(Player& player)override;

	void SetWalk(void);
	void SetRun(void);
};

