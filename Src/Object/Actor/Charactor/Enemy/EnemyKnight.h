#pragma once
#include "EnemyBase.h"

class EnemyKnight :
    public EnemyBase
{
public:

	// アニメーション種別
	enum class ANIM_TYPE
	{
		IDLE = 8,
		WALK = 12,
		RUN = 11,
		ROAR = 10,
		MELEE_ATTACK = 13,
		DIE = 4
	};

	// プレイヤー状態
	enum class STATE
	{
		IDLE,
		WALK,
		RUN,
		ROAR,
		MELEE_ATTACK,
		DIE
	};

    // コンストラクタ
	EnemyKnight(void);

    // デストラクタ
    virtual ~EnemyKnight(void) override;

protected:

	// プレイヤーリソースロード
	void InitLoad(void) override;

	// 大きさ、回転、座標のトランスフォーム初期化
	void InitTransform(void) override;

	// コライダー（線分・カプセル）の初期化
	void InitCollider(void) override;

	// アニメーションコントローラーの構築とアニメーション登録
	void InitAnimation(void) override;

	// ロード後のコンボパラメータや内部変数の初期化
	void InitPost(void) override;

	// プレイヤーの主更新処理
	void UpdateProcess(void) override;

	// プレイヤー更新後の後処理
	void UpdateProcessPost(void) override;

	// 当たり判定衝突時の更新処理
	void UpdateHitCollider(void) override;

private:
};

