#pragma once
#include "../../../../Utility/JsonUtility.h"
#include "EnemyBase.h"

class EnemyBoss :
	public EnemyBase
{

	friend class BossMoveState;

public:

	// アニメーション種別
	enum class ANIM_TYPE
	{
		IDLE,
		WALK,
		RUN,
		ATTACK,
	};

	// エネミー状態
	enum class STATE
	{
		IDLE,
		MOVE,
		CHSDR,
		ATTACK,
		DIE
	};

	// コンストラクタ
	EnemyBoss(void);

	// デストラクタ
	virtual ~EnemyBoss(void) override;

	// リソース解放
	void Release(void) override;

	// ランタイム用GUI描画
	void UpdateGui(void) override;

	// EnemyData.jsonへの直接参照を取得
	Json& GetData(void) const;

protected:

	// リソースロード
	void InitLoad(void) override;

	// 大きさ、回転、座標のトランスフォーム初期化
	void InitTransform(void) override;

	// コライダー（線分・カプセル）の初期化
	void InitCollider(void) override;

	// アニメーションコントローラーの構築とアニメーション登録
	void InitAnimation(void) override;

	// ロード後の初期化
	void InitPost(void) override;

	// 主更新処理
	void UpdateProcess(void) override;

	// 更新後の後処理
	void UpdateProcessPost(void) override;

	// 当たり判定衝突時の更新処理
	void UpdateHitCollider(void) override;

private:

	// JSONの変更をTransform・コライダー・アニメ速度に即時反映する
	void ApplyJson(void);
};
