#pragma once
#include <functional>
#include <initializer_list>
#include <vector>
#include "EnemyBlackboard.h"

class BossAl
{
public:

	// 遷移なしを表す状態ID
	static constexpr int INVALID_STATE = -1;

	// どの状態からでも評価する遷移元ID
	static constexpr int ANY_STATE = -1;

	// 再利用可能な条件式の型
	using Condition = std::function<bool(const EnemyBlackboard&)>;

	// コンストラクタ
	BossAl(void);

	// デストラクタ
	~BossAl(void);

	// 遷移条件ルールを初期化する
	void Init(void);

	// ブラックボードを評価し、次に遷移すべき状態IDを返す(遷移不要ならINVALID_STATE)
	int DecideNextState(const EnemyBlackboard& bb) const;

	// --- 条件式の合成ヘルパー ---
	static Condition And(const Condition& a, const Condition& b);
	static Condition Or(const Condition& a, const Condition& b);
	static Condition Not(const Condition& a);

	// --- 再利用可能な共通条件パーツ ---
	static Condition IsInFov(float halfAngleDeg);
	static Condition IsOutOfFov(float halfAngleDeg);
	static Condition IsDistLessEq(float dist);
	static Condition IsDistGreater(float dist);
	static Condition IsDistInRange(float minDist, float maxDist);
	static Condition IsHpRateLessEq(float rate);
	static Condition IsDead(void);
	static Condition IsActionFinished(void);
	static Condition IsStateElapsedGreaterEq(float sec);
	static Condition IsCoolTimeReady(int key);

private:

	// 遷移ルール
	struct TransitionRule
	{
		// 遷移元の状態ID(ANY_STATEなら全状態から評価)
		int fromState = ANY_STATE;
		// 遷移先の状態ID
		int toState = INVALID_STATE;
		// 優先度(大きいほど先に評価)
		int priority = 0;
		// 遷移条件式
		Condition condition;
	};

	// 遷移ルール一覧(優先度降順)
	std::vector<TransitionRule> rules_;

	// 単一の遷移元ステートに対してルールを追加
	void AddTransition(int fromState, int toState, int priority, const Condition& condition);

	// 複数の遷移元ステートに対して共通ルールを一括追加
	void AddTransition(
		std::initializer_list<int> fromStates,
		int toState,
		int priority,
		const Condition& condition);
};
