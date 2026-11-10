#include "../Manager/InputManager.h"
#include "../Manager/SceneManager.h"
#include "../Manager/CollisionManager.h"
#include "../Manager/EditorManager.h"
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
	bool isClose = ins.IsTrgDown(KEY_INPUT_2)
		|| ins.IsPadBtnNew(InputManager::JOYPAD_NO::PAD1, InputManager::JOYPAD_BTN::TOP);

	// デバッグシーン解放
	if (isClose)
	{
		sceMng_.PopScene();
		return;
	}
}

void DebugScene::UpdateGui(void)
{
#ifdef _DEBUG
	// 統合エディタのGUI更新・描画
	EditorManager::GetInstance().UpdateGui();
#endif
}

void DebugScene::Draw(void)
{
#ifdef _DEBUG
	// コライダー表示が有効な場合のみデバッグ描画
	if (EditorManager::GetInstance().GetIsDrawCollider())
	{
		CollisionManager::GetInstance().DrawDebug();
	}
#endif
}

void DebugScene::Release(void)
{
}
