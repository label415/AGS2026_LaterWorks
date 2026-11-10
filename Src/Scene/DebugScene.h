#pragma once
#include "SceneBase.h"
class DebugScene :
    public SceneBase
{
public:

	// コンストラクタ
	DebugScene(void);

	// デストラクタ
	~DebugScene(void) override;

	// 読み込み  
	void Load(void)override;

	// 読み込み後の初期化  
	void LoadEnd(void)override;

	// 更新  
	void Update(void) override;

	// GUI更新
	void UpdateGui(void) override;

	// 描画  
	void Draw(void) override;

	// 解放  
	void Release(void) override;

private:


};

