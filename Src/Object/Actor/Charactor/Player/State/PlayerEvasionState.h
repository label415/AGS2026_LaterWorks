#pragma once
#include "../../../../../Common/Quaternion.h"
#include "../../StateBase.h"

class Player;

class PlayerEvasionState : public StateBase<Player>
{
public:

    // コンストラクタ
    PlayerEvasionState(void);

    // 開始
    void StartState(Player& player)override;

    // 更新
    void Update(Player& player)override;

    // 終了
	void EndState(Player& player)override;

private:

    // 初期角度を保存
    Quaternion lastQrot_;
};

