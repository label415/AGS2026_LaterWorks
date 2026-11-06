#include "../Object/Actor/UI/UiBase.h"
#include "UiManager.h"

void UiManager::Add(std::unique_ptr<UiBase> ui)
{
	// UI登録
	uis_.push_back(std::move(ui));
}

void UiManager::Update(void)
{
	// UIが空の場合は処理しない
	if (uis_.empty())return;

	// 各UI更新
	for (const auto& ui : uis_)
	{
		// UIがnullptrの場合は処理しない
		if (ui == nullptr)continue;

		// UI更新
		ui->Update();
	}
}

void UiManager::Draw(void)
{
	// UIが空の場合は処理しない
	if (uis_.empty())return;

	// 各UI描画
	for (const auto& ui : uis_)
	{
		// UIがnullptrの場合は処理しない
		if (ui == nullptr)continue;

		// UI描画
		ui->Draw();
	}
}

void UiManager::Release(void)
{
	// UIが空の場合は処理しない
	if (uis_.empty())return;

	// 各UI解放
	for(const auto& ui : uis_)
	{
		// UIがnullptrの場合は処理しない
		if (ui == nullptr)continue;

		// UI解放
		ui->Release();
	}

	// 配列をクリア
	uis_.clear();
}

UiManager::UiManager(void)
{
	// 配列をクリア
	uis_.clear();
}