#include <string>
#include <fstream>
#include "../../../../Application.h"
#include "../../../../Utility/AsoUtility.h"
#include "../../../Common/Collider/ColliderBase.h"
#include "EnemyDragon.h"
#include "EnemyManager.h"

EnemyManager::EnemyManager(void){}

EnemyManager::~EnemyManager(void){}

void EnemyManager::Load(void)
{
	Create();
	// エネミーのロード
	for (auto& enemy : enemys_)
	{
		enemy->Load();
	}
}

void EnemyManager::LoadEnd(void)
{
	// エネミーの初期化
	for (auto& enemy : enemys_)
	{
		enemy->LoadEnd();
	}
}

void EnemyManager::Update(void)
{
	// エネミーの更新
	for (auto& enemy : enemys_)
	{
		enemy->Update();
	}
}

void EnemyManager::Draw(void)
{
	// エネミーの描画
	for (auto& enemy : enemys_)
	{
		enemy->Draw();
	}
}

void EnemyManager::Release(void)
{
	// エネミーの解放
	for (auto& enemy : enemys_)
	{
		enemy->Release();
	}
}

void EnemyManager::Create(void)
{
	auto enemy = std::make_shared<EnemyDragon>();
	enemys_.push_back(enemy);
}