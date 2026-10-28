#pragma once

class Player;

class PlayerStateBase
{
public:

	// コンストラクタ
	PlayerStateBase(void);

	// デストラクタ
	virtual ~PlayerStateBase(void) = 0;

	// 開始
	virtual void StartState(Player& player) = 0;

	// 更新
	virtual void Update(Player& player) = 0;

	// 終了
	virtual void EndState(Player& player) = 0;
};

