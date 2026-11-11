#include "../../../Manager/ResourceManager.h"
#include "../../../Manager/SceneManager.h"
#include "../../../Manager/FontManager.h"
#include "../../../Application.h"
#include "UiSt.h"

// リソース名
namespace
{
	// バーフレーム
	constexpr const wchar_t* IMG_BARFRAME_NAME = L"ImgBarFrame";
	// STバー
	constexpr const wchar_t* IMG_STBAR_NAME = L"ImgSTBar";
}

UiSt::UiSt(
	const float* stCurrent,
	float posX, float posY,
	float sclX, float sclY, float flameScl)
	: UiBase()
	,stCurrent_(stCurrent)
	, pos_( posX, posY )
	, flameScl_(flameScl)
	, stSclX_(sclX)
	, stSclY_(sclY)
	, flameImg_(-1)
	, suUiImg_(-1)
{
	// リソースのロード
	flameImg_ = ResourceManager::GetInstance()
		.Load(IMG_BARFRAME_NAME).handleId_;
	suUiImg_ = ResourceManager::GetInstance()
		.Load(IMG_STBAR_NAME).handleId_;
}

UiSt::~UiSt(void){}

void UiSt::Update(void){}

void UiSt::Draw(void)
{
	// 背景バーの描画
	DrawRotaGraph(
		pos_.x,
		pos_.y,
		flameScl_, 0.0f, flameImg_, true);

	// 現在のスタミナ値に応じたバーの描画
	float hpRate = *stCurrent_ / MAX_ST;
	float realHalfWidth = IMG_SIZE_X / stSclX_;
	float realHalfHeight = IMG_SIZE_Y / stSclY_;

	float left = pos_.x - realHalfWidth;
	float top = pos_.y - realHalfHeight;
	float bottom = pos_.y + realHalfHeight;

	int srcWidth = static_cast<int>(IMG_SIZE_X * hpRate);
	float currentBarRight = left + (realHalfWidth * 2.0f * hpRate);

	// HPバー
	DrawRectExtendGraphF(
		left, top,
		currentBarRight, bottom,
		0, 0, srcWidth,
		static_cast<int>(IMG_SIZE_Y),
		suUiImg_, true);
}

void UiSt::Release(void){}
