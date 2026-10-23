#include <DxLib.h>
#include "../Object/Actor/Charactor/Player.h"
#include "../Object/Actor/Charactor/Enemy/EnemyManager.h"
#include "../Object/Actor/Stage/Stage.h"
#include "../Object/Actor/Stage/SkyDome.h"
#include "../Object/Common/Collider/ColliderCapsule.h"
#include "../Common/ShadowMap.h"
#include "../Manager/Camera.h"
#include "../Manager/SceneManager.h"
#include "../Manager/InputManager.h"
#include "../Manager/FontManager.h"
#include "../Manager/ResourceManager.h"
#include "../Manager/SoundManager.h"
#include "../Manager/CollisionManager.h"
#include "../Application.h"
#include "GameScene.h"

GameScene::GameScene(void)
	:
	SceneBase()
{
}

GameScene::~GameScene(void) {}

void GameScene::Load(void)
{
	// カメラのインスタンス取得
	camera_ = SceneManager::GetInstance().GetCamera();
	camera_->ChangeMode(Camera::MODE::FIXED_POINT);

	// スカイドーム読み込み
	skydome_ = std::make_unique<SkyDome>();
	skydome_->Load();

	// ステージ初期化
	stage_ = std::make_unique<Stage>();
	stage_->Load();

	// プレイヤー読み込み
	player_ = std::make_shared<Player>();
	player_->Load();

	// エネミー読み込み
	enemys_ = std::make_shared<EnemyManager>();
	enemys_->Load();

	// シャドーマップ読み込み
	shadowMap_ = std::make_unique<ShadowMap>(SHADOW_MAP_RESOLUTION, SHADOW_MAP_RESOLUTION);

	// カメラモード変更
	camera_->SetFollow(&player_->GetTransform());
	camera_->ChangeMode(Camera::MODE::FOLLOW);

	// 入力管理のインスタンス取得
	camera_->SetIsMouseInput(true);
	InputManager::GetInstance().SetMouseFlage(false);
}

void GameScene::LoadEnd(void)
{
	// BGM再生
	bgm_ = resMng_.Load(ResourceManager::SRC::BGM_GAME).handleId_;
	volume_ = BGM_VOLUME;
	SoundManager::GetInstance().PlayBGM(bgm_, volume_);

	// スカイドーム初期化
	skydome_->LoadEnd();
	// ステージ初期化
	stage_->LoadEnd();
	// プレイヤー初期化
	player_->LoadEnd();
	// エネミー初期化
	enemys_->Init();

	// シャドーマップ初期化
	shadowMap_->AddShadowMapLight(GetLightDirection());
	VECTOR playerPos = player_->GetTransform().pos;
	shadowMap_->AddShadowMapDrawArea(
		VGet((playerPos.x - SHADOW_MAP_DIFF),
			SHADOW_MAP_MIN_DRAW,
			(playerPos.z - SHADOW_MAP_DIFF)),
		VGet(
			(playerPos.x + SHADOW_MAP_DIFF),
			SHADOW_MAP_DIFF * SHADOW_MAP_MAX_DRAW,
			(playerPos.z + SHADOW_MAP_DIFF))
	);
}

void GameScene::Update(void)
{
	// シーン変更
	UpdateResult();

	// Effekseerにより再生中のエフェクトを更新する。
	UpdateEffekseer3D();

	// 追尾対象更新
	UpdateAutoLockOn();

	// スカイドーム更新
	skydome_->Update();

	//ステージ更新
	stage_->Update();

	// エネミー更新
	enemys_->Update();

	// プレイヤー更新
	player_->Update();
	
	// シャドーマップ更新
	shadowMap_->AddShadowMapLight(GetLightDirection());
	VECTOR playerPos = player_->GetTransform().pos;
	shadowMap_->AddShadowMapDrawArea(
		VGet((playerPos.x - SHADOW_MAP_DIFF),
			SHADOW_MAP_MIN_DRAW,
			(playerPos.z - SHADOW_MAP_DIFF)),
		VGet((playerPos.x + SHADOW_MAP_DIFF),
			SHADOW_MAP_DIFF * SHADOW_MAP_MAX_DRAW,
			(playerPos.z + SHADOW_MAP_DIFF))
	);
}

void GameScene::Draw(void)
{
	// シャドウ描画のセットアップ
	shadowMap_->DrawSetup();
	ObjectDraw();
	// シャドウ描画の終了
	shadowMap_->DrawEnd();

	// シャドウマップを使用して描画する
	shadowMap_->SetShadow();
	ObjectDraw();
	// シャドウマップの使用を終了する
	shadowMap_->EndShadow();

	// Effekseerにより再生中のエフェクトを描画する。
	DrawEffekseer3D();

	// 各UI描画
	DrawUI();
}

void GameScene::ObjectDraw(void)
{
	// スカイドーム描画
	skydome_->Draw();

	// プレイヤー描画
	player_->Draw();

	// エネミー描画
	enemys_->Draw();

	// ステージ描画
	stage_->Draw();
}

void GameScene::Release(void)
{
	// スカイドーム解放
	skydome_->Release();

	// ステージ解放
	stage_->Release();

	//プレイヤー解放
	player_->Release();

	//各エネミー解放
	enemys_->Release();

	//シャドウマップ解放
	shadowMap_->Release();
}

void GameScene::UpdateAutoLockOn(void)
{
	std::shared_ptr<Camera> camera = SceneManager::GetInstance().GetCamera();

	// カメラ側でロックオン対象の選別・切り替えを更新
	camera->UpdateLockOn();

	// カメラが現在ロックオンしている対象のTransformをプレイヤーに渡す
	player_->SetTargetPos(camera->GetLockOnTargetPos());

	// 追尾対象追加
	for (auto& enemy : enemys_->GetEnemys())
	{
		enemy->SetTargetPos(&player_->GetTransform().pos);
		for (const auto& collider
			: enemy->GetOwnCollider(static_cast<int>(ColliderBase::SHAPE::CAPSULE)))
		{
			auto enemyCapsule = std::dynamic_pointer_cast<ColliderCapsule>(collider);
			camera_->AddTargetPos(&enemyCapsule->GetCenter());
		}
	}
}

void GameScene::UpdateResult(void)
{
	// ポーズメニューの表示切り替え
	bool isSelect = InputManager::GetInstance().IsTrgDown(KEY_INPUT_ESCAPE)
		|| InputManager::GetInstance().IsPadBtnTrgDown(
			InputManager::JOYPAD_NO::PAD1, InputManager::JOYPAD_BTN::START);
	if (isSelect)
	{
		sceMng_.PushScene(SceneManager::SCENE_ID::PAUSE);
	}
}

void GameScene::DrawUI(void)
{
	// プレイヤーとエネミーのHP描画
	player_->DrawHp();
	for (const auto& enemy : enemys_->GetEnemys()) {
		enemy->DrawHp();
	}
}
