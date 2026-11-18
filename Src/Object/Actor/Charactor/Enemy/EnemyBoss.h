#pragma once
#include "../../../../Utility/JsonUtility.h"
#include "../StateBase.h"
#include "EnemyBase.h"

class EnemyBoss : public EnemyBase
{

	friend class BossMoveState;

public:

	// 遷移判断材料
	struct Blackboard
	{
		// プレイヤーとの距離
		float distance = 0.0f;
		// 正面からの角度
		float angleToPlayer = 0.0f;
		// 残りHPの割合
		float hpRate = 1.0f;

		// 現在の状態
		int currentState = 0;
		// 現在の相手の状態
		int currentStatePlayer = 0;
		// 過去に相手が行った状態
		std::vector<int> currentStatesPlayer;
	};

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

	// 遷移判断材料
	Blackboard blackboard_;

	// 各状態クラスのインスタンスを保持するマップ
	std::map<int, std::unique_ptr<StateBase<EnemyBoss>>> states_;

	// 状態クラス
	StateBase<EnemyBoss>* currentState_;

	// 状態
	int state_;

	// 状態遷移
	void ChangeState(int state);

	// JSONの変更をTransform・コライダー・アニメ速度に即時反映する
	void ApplyJson(void);

	// ステート初期化
	void InitState(void);

	// 遷移判断材料設定
	void SetBlackboard(void);
};
