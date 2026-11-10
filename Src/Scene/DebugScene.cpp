#include "../Manager/InputManager.h"
#include "../Manager/SceneManager.h"
#include "../Manager/CollisionManager.h"
#include "../Libs/ImGuiWrapper.h"
#include "DebugScene.h"

DebugScene::DebugScene(void)
{
}

DebugScene::~DebugScene(void)
{
}

void DebugScene::Load(void)
{
}

void DebugScene::LoadEnd(void)
{
}

void DebugScene::Update(void)
{
	// 入力管理インスタンスを取得
	auto& ins = InputManager::GetInstance();
	bool isDecide = ins.IsTrgDown(KEY_INPUT_2)
		|| ins.IsPadBtnNew(InputManager::JOYPAD_NO::PAD1, InputManager::JOYPAD_BTN::TOP);

	// シーン解放
	if (isDecide)
	{
		sceMng_.PopScene();
		return;
	}


}

void DebugScene::UpdateGui(void)
{
}

void DebugScene::Draw(void)
{
	// コライダー描画
	auto& colMng = CollisionManager::GetInstance();
	colMng.DrawDebug();
}

void DebugScene::Release(void)
{
}
