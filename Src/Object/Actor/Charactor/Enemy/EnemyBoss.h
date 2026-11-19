#pragma once
#include "../../../../Utility/JsonUtility.h"
#include "../StateBase.h"
#include "AI/EnemyBlackboard.h"
#include "AI/HfsmController.h"
#include "EnemyBase.h"

class EnemyBoss : public EnemyBase
{

	friend class BossMoveState;
	friend class BossChaseState;
	friend class BossAttackState;
	friend class BossIdleState;

public:

	static constexpr float FOV_HALF_ANGLE = 20.0f;

	// 遷移判断材料(他エネミーと共通のBlackboardを使用)
	using Blackboard = EnemyBlackboard;

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

	// エネミー状態(HFSMの葉ステートID)
	enum class STATE
	{
		IDLE,
		MOVE,
		CHSDR,
		ATTACK,
		DIE
	};

	// HFSMの階層グループID(STATEと重複しない値)
	enum class STATE_GROUP
	{
		ALIVE = 100, // 生存中の全状態
		NEUTRAL,     // 通常行動(待機・移動・旋回)
		COMBAT,      // 攻撃行動(攻撃・コンボ等)
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

	// 階層型ステートマシン(HFSM + Condition + Utility)
	HfsmController<EnemyBoss, Blackboard> hfsm_;

	// 状態遷移
	void ChangeState(int state);

	// JSONの変更をTransform・コライダー・アニメ速度に即時反映する
	void ApplyJson(void);

	// ステート・階層グループ初期化
	void InitState(void);

	// 遷移条件(Condition)とUtility選択ルールの一括登録
	void InitTransitions(void);

	// 遷移判断材料設定
	void SetBlackboard(void);

	// 追尾方向取得
	void SetTargetDir(void);
};
