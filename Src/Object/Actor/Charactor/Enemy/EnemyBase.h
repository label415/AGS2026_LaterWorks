#pragma once
#include <DxLib.h>
#include <functional>
#include "../CharactorBase.h"

class WeponBase;

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
    const int GetState(void)const { return stateBase_; }

protected:

    // 状態管理
    std::map<int, std::function<void(void)>> stateChanges_;

    // 状態管理(
    std::function<void(void)> stateUpdate_;

    // 種別
    TYPE type_;

    // HP
    int hp_;

    // 状態管理
    int stateBase_;

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

    // 状態遷移
    void ChangeState(int state);

    // 更新系
    virtual void UpdateProcessPost(void) override {}

};
