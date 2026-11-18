#pragma once
#include "../../StateBase.h"

class Player;

class PlayerIdleState : public StateBase<Player>
{
public:

	// コンストラクタ
	PlayerIdleState(void);

	// 開始
	void StartState(Player& player)override;

	// 更新
	void Update(Player& player)override;

	// 終了
	void EndState(Player& player)override;

};

