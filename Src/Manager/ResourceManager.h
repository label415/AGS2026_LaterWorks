#pragma once
#include <map>
#include <string>
#include "Resource.h"

class ResourceManager
{
public:

	// 明示的にインステンスを生成する
	static void CreateInstance(void);

	// 静的インスタンスの取得
	static ResourceManager& GetInstance(void);

	// 初期化
	void Init(void);

	// 解放(シーン切替時に一旦解放)
	void Release(void);

	// リソースの完全破棄
	void Destroy(void);

	// リソースのロード
	const Resource& Load(std::wstring src);

	// リソースの複製ロード(モデル用)
	int LoadModelDuplicate(std::wstring src);

	// JSONデータの取得(編集用)
	Json& GetJson(std::wstring src);

	// JSONデータのファイル保存
	bool SaveJson(std::wstring src);

	// JSONデータのファイル再読み込み
	bool ReloadJson(std::wstring src);

private:

	// 静的インスタンス
	static ResourceManager* instance_;

	// リソース管理の対象
	std::map<std::wstring, Resource*> resourcesMap_;

	// 読み込み済みリソース
	std::map<std::wstring, Resource&> loadedMap_;

	// リソース
	Resource dummy_;

	// デフォルトコンストラクタをprivateにして、
	// 外部から生成できない様にする
	ResourceManager(void);
	ResourceManager(const ResourceManager& manager) = default;
	~ResourceManager(void) = default;

	// 内部ロード
	Resource& _Load(std::wstring src);
};
