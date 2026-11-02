#pragma once
#include "PlayerStateBase.h"
class PlayerAttackState : public PlayerStateBase
{
public:

	// コンストラクタ
	PlayerAttackState(void);

	// デストラクタ
	~PlayerAttackState(void) override;

	// 開始
	void StartState(Player& player)override;

	// 更新
	void Update(Player& player)override;

	// 終了
	void EndState(Player& player)override;

};

