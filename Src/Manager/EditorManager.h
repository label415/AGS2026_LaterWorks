#pragma once
#include <string>
#include <vector>
#include <functional>
#include "../Template/Singleton.h"
#include "ResourceManager.h"

class EditorManager : public Singleton<EditorManager>
{
	friend class Singleton<EditorManager>;
public:

	// 編集対象オブジェクトの登録情報(JSON直接編集・保存方式)
	struct EditObject
	{
		// 一覧に表示するオブジェクト名
		std::string name;
		// 直接編集・保存するJSONリソース名
		std::wstring jsonSrc;
		// JSONの値が変更された時・再読込された時にゲーム側へ即時反映するコールバック
		std::function<void(void)> onApplyJson;
		// ランタイム状態(現在座標や現在HPなど)の追加GUI描画コールバック(任意)
		std::function<void(void)> drawRuntimeGui;
	};

	// 編集対象オブジェクトの登録
	void RegisterObject(const EditObject& obj);

	// 指定名の編集対象オブジェクトを解除
	void UnregisterObject(const std::string& name);

	// 登録されている全オブジェクトをクリア
	void ClearObjects(void);

	// 統合エディタUIの更新・描画
	void UpdateGui(void);

	// リソース解放
	void Release(void);

	// エディタ表示中にゲーム更新を一時停止するか
	bool GetIsPauseGame(void) const { return isPauseGame_; }

	// デバッグコライダーを描画するか
	bool GetIsDrawCollider(void) const { return isDrawCollider_; }

private:

	// ステータスメッセージ表示時間(秒)
	static constexpr float STATUS_DISPLAY_TIME = 3.0f;

	// 左ペイン(オブジェクト一覧)の横幅
	static constexpr float LEFT_PANE_WIDTH = 170.0f;

	// 登録されている編集対象リスト
	std::vector<EditObject> objects_;

	// 現在選択中のオブジェクトインデックス
	int selectedIndex_;

	// ゲーム一時停止フラグ
	bool isPauseGame_;

	// コライダー描画フラグ
	bool isDrawCollider_;

	// ステータスメッセージ(Save/Reload完了通知用)
	std::string statusMessage_;

	// ステータスメッセージ表示タイマー
	float statusTimer_;

	// コンストラクタ
	EditorManager(void);

	// デストラクタ
	~EditorManager(void) = default;

	// JSONデータを直接ImGuiウィジェットとして再帰描画・編集する(変更があればtrue)
	bool DrawJsonEditor(Json& json);
	bool DrawJsonNode(const std::string& key, Json& node, bool isTopLevel);
};
