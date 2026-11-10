#include <DxLib.h>
#include "../Scene/Loading/Loading.h"
#include "../Scene/TitleScene.h"
#include "../Scene/GameScene.h"
#include "../Scene/PauseScene.h"
#include "../Scene/DebugScene.h"
#include "../Manager/SoundManager.h"
#include "../Manager/InputManager.h"
#include "../Manager/CollisionManager.h"
#include "../Manager/UiManager.h"
#include "../Manager/EditorManager.h"
#include "../Application.h"
#include "Camera.h"
#include "ResourceManager.h"
#include "SceneManager.h"

SceneManager::SceneManager(void)
	: sceneId_(SCENE_ID::TITLE)
	, nextSceneId_(SCENE_ID::TITLE)
	, request_(REQUEST::NONE)
	, isLoadStart_(false)
{
}

void SceneManager::Init(void)
{
	// ロード画面生成
	load_ = std::make_unique<Loading>();
	load_->Load();

	// カメラ
	camera_ = std::make_shared<Camera>();
	camera_->LoadEnd();

	// 3D用の設定
	Init3D();

	// 初期シーン(ロード画面を経由して読み込む)
	ExecuteChangeScene(SCENE_ID::GAME);
}

void SceneManager::Init3D(void)
{
	// 背景色設定
	SetBackgroundColor(
		BACKGROUND_COLOR_R,
		BACKGROUND_COLOR_G,
		BACKGROUND_COLOR_B);

	// Zバッファを有効にする
	SetUseZBuffer3D(true);
	// Zバッファへの書き込みを有効にする
	SetWriteZBuffer3D(true);
	// バックカリングを有効にする
	SetUseBackCulling(true);

	// ライトの設定
	SetUseLighting(true);
	// ライトの方向
	ChangeLightTypeDir(LIGHT_DIRECTION);
	// ライトのディフューズカラー
	SetLightDifColor(LIGHT_DIFF_COLOR);
	// ライトのアンビエントカラー
	SetLightAmbColor(LIGHT_AMB_COLOR);

	// フォグ設定
	SetFogEnable(true);
	// フォグの色
	SetFogColor(
		FOG_COLOR_R,
		FOG_COLOR_G,
		FOG_COLOR_B);
	// フォグ表示範囲
	SetFogStartEnd(FOG_START, FOG_END);
}

void SceneManager::Update(void)
{
	// デバッグ用の入力処理
	UpdateDebugInput();

	// ロード中はシーンを更新しない
	if (UpdateLoading()) return;

	// デバッグシーン展開中かつ一時停止していない場合は、下のシーン(GameScene等)も更新する
	if (sceneId_ == SCENE_ID::DEBUG
		&& scenes_.size() >= 2
		&& !EditorManager::GetInstance().GetIsPauseGame())
	{
		scenes_[scenes_.size() - 2]->Update();
	}

	// 最上位シーンの更新
	if (!scenes_.empty())
	{
		scenes_.back()->Update();
	}

#ifdef _DEBUG
	// デバッグシーン展開中はGUIを更新する
	if (sceneId_ == SCENE_ID::DEBUG)
	{
		for (auto& scene : scenes_)
		{
			scene->UpdateGui();
		}
	}
#endif

	// カメラ更新
	camera_->Update();

	// シーン更新中に受け付けた遷移・解放要求を実行する
	ProcessRequest();
}

void SceneManager::Draw(void)
{
	// 描画先グラフィック領域を裏画面に指定
	SetDrawScreen(DX_SCREEN_BACK);

	// 画面を初期化する
	ClearDrawScreen();

	// ロード中はロード画面のみ描画する
	if (load_->GetIsLoading())
	{
		load_->Draw();
		return;
	}

	// カメラの描画前設定を行う
	camera_->SetBeforeDraw();

	// 下に積まれているシーンから順に描画する
	for (auto& scene : scenes_)
	{
		scene->Draw();
	}

	// カメラのデバッグ描画
	camera_->DrawDebug();
}

void SceneManager::Release(void)
{
	// 残っているシーンの解放
	ReleaseAllScenes();

	// カメラの解放
	camera_->Release();

	// ロード画面の解放
	load_->Release();
}

void SceneManager::ChangeScene(SCENE_ID sceneId)
{
	// 実行中のシーンを破棄しないよう、要求だけ受け付ける
	nextSceneId_ = sceneId;
	request_ = REQUEST::CHANGE;
}

void SceneManager::PushScene(SCENE_ID sceneId)
{
	// ロード中または既にサブシーンが積まれている場合は受け付けない
	if (load_->GetIsLoading()
		|| sceneIds_.size() > 1) return;

	// シーンを積んで、即座に読み込みと初期化を行う
	AddScene(sceneId);
	scenes_.back()->Load();
	scenes_.back()->LoadEnd();

	// サブシーン(ポーズ・デバッグ)中はマウスカーソルを表示する
	SetGameMouseMode(false);
}

void SceneManager::PopScene(void)
{
	// シーン遷移の要求が優先(遷移時に全シーン解放されるため)
	if (request_ == REQUEST::CHANGE) return;

	// 実行中のシーンを破棄しないよう、要求だけ受け付ける
	request_ = REQUEST::POP;
}

std::unique_ptr<SceneBase> SceneManager::CreateScene(SCENE_ID sceneId)
{
	// シーンIDに応じたインスタンスを生成して返す
	switch (sceneId)
	{
	case SCENE_ID::TITLE:
		return std::make_unique<TitleScene>();
	case SCENE_ID::GAME:
		return std::make_unique<GameScene>();
	case SCENE_ID::PAUSE:
		return std::make_unique<PauseScene>();
	case SCENE_ID::DEBUG:
		return std::make_unique<DebugScene>();
	default:
		return nullptr;
	}
}

void SceneManager::AddScene(SCENE_ID sceneId)
{
	// シーンとIDを同じ並びで積む
	scenes_.push_back(CreateScene(sceneId));
	sceneIds_.push_back(sceneId);

	// 現在のシーンIDを最上位のシーンに合わせる
	sceneId_ = sceneId;
}

void SceneManager::UpdateDebugInput(void)
{
#ifdef _DEBUG
	// デバッグシーンを開く入力
	auto& ins = InputManager::GetInstance();
	bool isOpenDebug = ins.IsTrgDown(KEY_INPUT_1)
		|| ins.IsPadBtnNew(InputManager::JOYPAD_NO::PAD1, InputManager::JOYPAD_BTN::TOP);

	// デバッグシーンを積む
	if (isOpenDebug)
	{
		PushScene(SCENE_ID::DEBUG);
	}
#endif
}

bool SceneManager::UpdateLoading(void)
{
	// ロード画面の更新
	load_->Update();

	// ロード画面を表示した次のフレームで、非同期読み込みを行う
	if (isLoadStart_)
	{
		isLoadStart_ = false;
		load_->StartAsyncLoad();
		scenes_.back()->Load();
		load_->EndAsyncLoad();
		return true;
	}

	// ロードが終わったフレームで一度だけ、読み込み後の初期化を行う
	if (load_->GetIsLoadEnd())
	{
		scenes_.back()->LoadEnd();
	}

	return load_->GetIsLoading();
}

void SceneManager::ProcessRequest(void)
{
	// 要求を取り出してクリアする
	REQUEST request = request_;
	request_ = REQUEST::NONE;

	switch (request)
	{
	case REQUEST::CHANGE:
		ExecuteChangeScene(nextSceneId_);
		break;
	case REQUEST::POP:
		ExecutePopScene();
		break;
	default:
		break;
	}
}

void SceneManager::ExecuteChangeScene(SCENE_ID sceneId)
{
	// 積まれている全シーンを解放する
	ReleaseAllScenes();

	// サウンド・リソース・当たり判定・カメラのリセット
	ResetResource();

	// 新しいシーンを生成(読み込みは次のフレームで行う)
	AddScene(sceneId);

	// ロード画面の表示開始
	load_->StartLoading();
	isLoadStart_ = true;
}

void SceneManager::ExecutePopScene(void)
{
	// 最後の1シーンは残す
	if (scenes_.size() <= 1) return;

	// 最上位のシーンを解放して削除する
	scenes_.back()->Release();
	scenes_.pop_back();
	sceneIds_.pop_back();

	// 現在のシーンIDを、下にあったシーンに戻す
	sceneId_ = sceneIds_.back();

	// ポーズ・デバッグから戻った際はゲーム操作用のマウス設定に戻す
	SetGameMouseMode(true);
}

void SceneManager::ReleaseAllScenes(void)
{
	// 積まれている全シーンを解放する
	for (auto& scene : scenes_)
	{
		scene->Release();
	}
	scenes_.clear();
	sceneIds_.clear();
}

void SceneManager::ResetResource(void)
{
	// サウンド・リソース・当たり判定・UI・エディタの解放
	EditorManager::GetInstance().Release();
	SoundManager::GetInstance().Release();
	ResourceManager::GetInstance().Release();
	CollisionManager::GetInstance().Release();
	UiManager::GetInstance().Release();

	// エフェクトの再初期化
	Application::GetInstance().InitEffekseer();

	// カメラの解放
	camera_->Release();

	// ロード中・メニュー操作用のマウス設定にする
	SetGameMouseMode(false);
}

void SceneManager::SetGameMouseMode(bool isGame)
{
	// ゲーム中はカメラをマウス操作し、カーソルを非表示にする
	camera_->SetIsMouseInput(isGame);
	InputManager::GetInstance().SetMouseFlage(!isGame);
}
