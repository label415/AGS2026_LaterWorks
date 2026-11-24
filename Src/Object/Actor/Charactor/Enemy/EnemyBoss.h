#pragma once
#include "../../../../Utility/JsonUtility.h"
#include "EnemyBlackboard.h"
#include "EnemyBase.h"

class EnemyStateManager;

class EnemyBoss : public EnemyBase
{

	friend class BossMoveState;
	friend class BossChaseState;
	friend class BossAttackState;
	friend class BossIdleState;

public:

	static constexpr float FOV_HALF_ANGLE = 20.0f;

	// アニメーション種別
	enum class ANIM_TYPE
	{
		IDLE,
		WALK,
		RUN,
		LEFT_ROT,
		RIGHT_ROT,
		ATTACK,
		COMBO,
		DAMAGE,
	};

	// エネミー状態
	enum class STATE
	{
		IDLE,
		WALK,
		RUN,
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

	// ブラックボードを取得
	EnemyBlackboard& GetBlackboard(void) { return blackBoard_; }
	const EnemyBlackboard& GetBlackboard(void) const { return blackBoard_; }

	// 追尾対象(プレイヤー)の状態参照を設定
	void SetTargetState(const int* targetState) { targetState_ = targetState; }

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

	// 過去のプレイヤー状態履歴の最大保持数
	static constexpr size_t MAX_PLAYER_STATE_HISTORY = 10;

	// ステート管理
	std::unique_ptr<EnemyStateManager> states_;

	// AI判断材料(ブラックボード)
	EnemyBlackboard blackBoard_;

	// 追尾対象(プレイヤー)の状態ポインタ
	const int* targetState_;

	// JSONの変更をTransform・コライダー・アニメ速度に即時反映する
	void ApplyJson(void);

	// ステート・階層グループ初期化
	void InitState(void);

	// ブラックボード更新
	void SetBlackboard(void);

	// 追尾方向取得
	void SetTargetDir(void);
};
