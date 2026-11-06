#pragma once
#include "../../../Template/Vector2Template.h"
#include "UiBase.h"

class UiSt : public UiBase
{
public:

    // コンストラクタ
    UiSt(const float* stCurrent
        , float posX, float posY
        , float sclX, float sclY, float flameScl);

    // デストラクタ
    ~UiSt(void)override;

    // 更新
    void Update(void)override;

    // 描画
    void Draw(void)override;

    // 解放
    void Release(void)override;

private:

    //スタミナ最大値
    static constexpr float MAX_ST = 100.0f;

    //画像サイズ
    static constexpr int IMG_SIZE_X = 866;
    static constexpr int IMG_SIZE_Y = 288;

    // スタミナの座標
    Vector2 pos_;

	// 画像ハンドルID
    int flameImg_;
    int suUiImg_;
    
    // スタミナの値
    const float* stCurrent_;

    // 枠のスケール
    float flameScl_;

    // スタミナバーのスケール
    float stSclX_;
    float stSclY_;
};


