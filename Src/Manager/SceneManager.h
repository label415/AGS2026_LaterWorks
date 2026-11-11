#pragma once
#include <vector>
#include <memory>
#include <DxLib.h>
#include "../Template/Singleton.h"
#include "../Template/Vector2Template.h"

class SceneBase;
class Camera;
class Loading;

class SceneManager : public Singleton<SceneManager>
{
	friend class Singleton<SceneManager>;
public:
	// シーン名
	enum class SCENE_ID
	{
		TITLE,
		GAME,
		PAUSE,
		DEBUG
	};

	// 背景色
	static constexpr int BACKGROUND_COLOR_R = 0;
	static constexpr int BACKGROUND_COLOR_G = 0;
	static constexpr int BACKGROUND_COLOR_B = 0;

	// ディレクショナルライトの方向
	static constexpr VECTOR LIGHT_DIRECTION = { -0.5f, -1.0f, -0.5f };
	// ライトのディフューズカラー値
	static constexpr COLOR_F LIGHT_DIFF_COLOR = { 1.0f, 1.0f, 1.0f, 1.0f };
	// ライトのアンビエントカラー値
	static constexpr COLOR_F LIGHT_AMB_COLOR = { 1.0f, 1.0f, 1.0f, 1.0f };

	// フォグカラー値
	static constexpr int FOG_COLOR_R = 5;
	static constexpr int FOG_COLOR_G = 5;
	static constexpr int FOG_COLOR_B = 5;

	// フォグ表示範囲
	static constexpr float FOG_START = 10000.0f;
	static constexpr float FOG_END = 20000.0f;

	// デルタタイム(60FPS固定)
	static constexpr float DELTA_TIME = 1.0f / 60.0f;

	// 初期化
	void Init(void);

	// 3Dの初期化
	void Init3D(void);

	// 更新
	void Update(void);

	// 描画
	void Draw(void);

	// リソースの破棄
	void Release(void);

	// シーン遷移(要求のみ受け付け、Updateの最後に実行する)
	void ChangeScene(SCENE_ID sceneId);

	// シーン追加(現在のシーンの上に積む)
	void PushScene(SCENE_ID sceneId);

	// シーン解放(要求のみ受け付け、Updateの最後に実行する)
	void PopScene(void);

	// デルタタイムの取得
	float GetDeltaTime(void) const { return DELTA_TIME; }

	// カメラの取得
	std::shared_ptr<Camera> GetCamera(void) const { return camera_; }

	// 現在のシーンID
	const SCENE_ID GetSceneID(void) const { return sceneId_; }

private:
	// シーン操作の要求種別
	enum class REQUEST
	{
		NONE,	// 要求なし
		CHANGE,	// シーン遷移
		POP,	// 最上位シーンの解放
	};

	// 現在のシーンID(最上位シーンのID)
	SCENE_ID sceneId_;

	// 次に遷移するシーンID
	SCENE_ID nextSceneId_;

	// 保留中のシーン操作要求
	REQUEST request_;

	// ロード画面を表示した次のフレームで読み込みを開始するフラグ
	bool isLoadStart_;

	// 積まれているシーン
	std::vector<std::unique_ptr<SceneBase>> scenes_;

	// 積まれているシーンのID(scenes_と同じ並び)
	std::vector<SCENE_ID> sceneIds_;

	// カメラ
	std::shared_ptr<Camera> camera_;

	// ロード画面
	std::unique_ptr<Loading> load_;

	// デフォルトコンストラクタをprivateにして、外部から生成できない様にする
	SceneManager(void);

	// コピーコンストラクタ
	SceneManager(const SceneManager& instance) = default;

	// デストラクタ
	~SceneManager(void) = default;

	// 指定されたシーンIDのインスタンスを生成する
	std::unique_ptr<SceneBase> CreateScene(SCENE_ID sceneId);

	// シーンを最上位に積む
	void AddScene(SCENE_ID sceneId);

	// デバッグ用の入力処理
	void UpdateDebugInput(void);

	// ロード処理の更新(ロード中はtrueを返す)
	bool UpdateLoading(void);

	// 保留中のシーン操作要求を実行する
	void ProcessRequest(void);

	// シーン遷移の実行
	void ExecuteChangeScene(SCENE_ID sceneId);

	// 最上位シーン解放の実行
	void ExecutePopScene(void);

	// 積まれている全シーンの解放
	void ReleaseAllScenes(void);

	// サウンド・リソース・当たり判定・カメラ等のリセット
	void ResetResource(void);

	// ゲーム操作用のマウス設定(カーソル非表示・カメラ操作有効)
	void SetGameMouseMode(bool isGame);
};
