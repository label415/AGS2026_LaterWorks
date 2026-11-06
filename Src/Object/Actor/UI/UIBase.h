#pragma once
#include "../../../Template/Vector2Template.h"

class UiBase
{
public:

    // コンストラクタ
    UiBase(void);

    // デストラクタ
    virtual ~UiBase(void) = default;

    // 更新
    virtual void Update(void){}

    // 描画
    virtual void Draw(void) = 0;

    // 解放
    virtual void Release(void) = 0;

    // 有効・無効判定
    bool IsActive(void) const { return isAlive_; }

    // 有効・無効設定
    void SetActive(bool active){ isAlive_  = active; }

protected:
  
    // 生存フラグ
    bool isAlive_;

};

