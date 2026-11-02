#pragma once
#include "PlayerStateBase.h"

class PlayerIdleState : public PlayerStateBase
{
public:

	// コンストラクタ
	PlayerIdleState(void);

	// デストラクタ
	~PlayerIdleState(void) override;

	// 開始
	void StartState(Player& player)override;

	// 更新
	void Update(Player& player)override;

	// 終了
	void EndState(Player& player)override;

};

