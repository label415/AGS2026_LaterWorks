#include "../../../Manager/ResourceManager.h"
#include "../../../Manager/SceneManager.h"
#include "../../../Manager/FontManager.h"
#include "../../../Application.h"
#include "UiHp.h"

UiHp::UiHp(const float* hpCurrent,
	float posX, float posY,
	float sclX, float sclY, float flameScl)
    : UiBase()
	, hpCurrent_(hpCurrent)
    , uiPos_(posX, posY)
    , hpSclX_(sclX)
    , hpSclY_(sclY)
	, flameScl_(flameScl)
	, flameImg_(-1)
	, hpImg_(-1)
{
	// リソースのロード
	flameImg_ = ResourceManager::GetInstance()
		.Load(ResourceManager::SRC::UI_BAR_FRAME).handleId_;
	hpImg_ = ResourceManager::GetInstance()
		.Load(ResourceManager::SRC::UI_HP_BAR).handleId_;
}

UiHp::~UiHp(void){}

void UiHp::Update(void){}

void UiHp::Draw(void)
{
	// 背景バーの描画
	DrawRotaGraph(
		uiPos_.x, uiPos_.y,
		flameScl_, 0.0f, flameImg_, true);

	// HPバーの描画
	float hpRate = *hpCurrent_ / MAX_HP;
	float realHalfWidth  = IMG_SIZE_X / hpSclX_;
	float realHalfHeight = IMG_SIZE_Y / hpSclY_;

	float left   = uiPos_.x - realHalfWidth;
	float top    = uiPos_.y - realHalfHeight;
	float bottom = uiPos_.y + realHalfHeight;

	int srcWidth = static_cast<int>(IMG_SIZE_X * hpRate);
	float currentBarRight = left + (realHalfWidth * 2.0f * hpRate);

	// HPバー
	DrawRectExtendGraphF(
		left, top,
		currentBarRight, bottom, 
		0, 0, srcWidth, 
		static_cast<int>(IMG_SIZE_Y),
		hpImg_, true);
}

void UiHp::Release(void){}