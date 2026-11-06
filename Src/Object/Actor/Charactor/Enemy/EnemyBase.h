#pragma once
#include <DxLib.h>
#include <functional>
#include "../CharactorBase.h"

class WeponBase;
class EnemyStateBase;

class EnemyBase :
    public CharactorBase
{
public:

    // 種別
    enum class TYPE
    {
        DRAGON,
    };

    // コンストラクタ
    EnemyBase(void);

    // デストラクタ
    virtual ~EnemyBase(void) override;

    // 描画
    virtual void Draw(void) override;

	// 種別取得
    const int GetState(void)const { return state_; }

protected:

    //// 各状態クラスのインスタンスを保持するマップ
    //std::map<int, std::unique_ptr<EnemyStateBase>> states_;

    //// 現在の状態クラス
    //EnemyStateBase* currentState_;

    // 状態
    int state_;

    // リソースロード
    void InitLoad(void) override {}

    // 大きさ、回転、座標の初期化
    void InitTransform(void) override {}

    // 衝突判定の初期化
    void InitCollider(void) override {}

    // アニメーションの初期化
    void InitAnimation(void) override {}

    // 初期化後の個別処理
    void InitPost(void) override {}

    // 更新系
    virtual void UpdateProcessPost(void) override {}

    // 状態遷移
    void ChangeState(int state);

};
