#pragma once
#include "../../../../../Common/Quaternion.h"
#include "PlayerStateBase.h"

class PlayerEvasionState :
    public PlayerStateBase
{
public:

    // コンストラクタ
    PlayerEvasionState(void);

    // デストラクタ
    ~PlayerEvasionState(void) override;

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

