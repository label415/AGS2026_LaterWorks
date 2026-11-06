#pragma once
#include <vector>
#include <memory>
#include "../Template/Singleton.h"

class UiBase;

class UiManager : public Singleton<UiManager>
{

	friend class Singleton<UiManager>;

public:

	// 登録
	void Add(std::unique_ptr<UiBase> ui);

	// 更新
	void Update(void);

	// 描画
	void Draw(void);

	// 解放
	void Release(void);

private:

	// UI情報を格納する動的配列
	std::vector<std::unique_ptr<UiBase>> uis_;

	// デフォルトコンストラクタをprivateにして、外部から生成できない様にする
	UiManager(void);

	// コピーコンストラクタ
	UiManager(const UiManager& instance) = default;

	// デストラクタ
	~UiManager(void) = default;

};


