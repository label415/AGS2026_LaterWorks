#pragma once
#include <string>
#include "UIBase.h"

class UiRecovery : public UiBase
{
public:

    // コンストラクタ
    UiRecovery(const float* stCurrent
        , float posX, float posY
        , float sclX, float sclY);

    // デストラクタ
    ~UiRecovery(void)override;

    // 更新
    void Update(void)override;

    // 描画
    void Draw(void)override;

    // 解放
    void Release(void)override;

private:

    //　使用するフォント
    std::wstring fontName_ = L"KazukiReiwa";
    // フォントのサイズ
    static constexpr int FONT_SIZE = 50;

    // アイテムボックスの大きさ
	static constexpr float BOX_SIZE = 0.8f;

	// 回復瓶の大きさ
	static constexpr float BOTTLE_SIZE = 0.4f;

	// UIの座標調整値
	static constexpr float UI_POS_ADJ_X = 8.0f;
    static constexpr float UI_POS_ADJ_Y = 1.5f;

    // フォント座標調整値
	static constexpr float FONT_POS_ADJ_X = 6.5f;
    static constexpr float FONT_POS_ADJ_Y = 1.5f;

	// フォントの色
	static constexpr int FONT_COLOR = 0xffffff;

    // 回復瓶の数
    int bottleCount_;

	// 回復瓶の数を表示するフォント
    std::wstring cntfont_;

    // アイテムボックスハンドル
    int imgBox_;

	// 回復瓶ハンドル
    int imgBottle_;

    // フォント
    int font_;

};

