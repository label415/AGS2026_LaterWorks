#include <algorithm>
#include "../EnemyBoss.h"
#include "BossAl.h"

namespace
{
	// 行動判定の距離閾値
	constexpr float DIST_ATTACK_RANGE = 250.0f; // 攻撃開始距離
	constexpr float DIST_WALK_STOP    = 350.0f; // 接近終了距離(待機へ戻る)
	constexpr float DIST_WALK_START   = 450.0f; // 歩き開始距離
	constexpr float DIST_RUN_STOP     = 650.0f; // 走り→歩きへ落とす距離
	constexpr float DIST_RUN_START    = 800.0f; // 走り開始距離
}

BossAl::BossAl(void)
{
}

BossAl::~BossAl(void)
{
}

void BossAl::Init(void)
{
	rules_.clear();

	constexpr int ST_IDLE   = static_cast<int>(EnemyBoss::STATE::IDLE);
	constexpr int ST_WALK   = static_cast<int>(EnemyBoss::STATE::WALK);
	constexpr int ST_RUN    = static_cast<int>(EnemyBoss::STATE::RUN);
	constexpr int ST_CHASE  = static_cast<int>(EnemyBoss::STATE::CHSDR);
	constexpr int ST_ATTACK = static_cast<int>(EnemyBoss::STATE::ATTACK);
	constexpr int ST_DIE    = static_cast<int>(EnemyBoss::STATE::DIE);

	// --- 再利用する共通条件式を作成 ---
	auto isDead           = IsDead();
	auto isActionFinished = IsActionFinished();
	auto isInFov          = IsInFov(EnemyBoss::FOV_HALF_ANGLE);
	auto isOutFov         = Not(isInFov);
	auto isAttackRange    = IsDistLessEq(DIST_ATTACK_RANGE);
	auto isWalkStop       = IsDistLessEq(DIST_WALK_STOP);
	auto isWalkStart      = IsDistGreater(DIST_WALK_START);
	auto isRunStop        = IsDistLessEq(DIST_RUN_STOP);
	auto isRunStart       = IsDistGreater(DIST_RUN_START);
	auto canAttack        = And(isInFov, And(isAttackRange, IsCoolTimeReady(ST_ATTACK)));

	// --- 1. 旋回・移動の終了条件(優先度高) ---
	// 旋回中：対象を正面(視野角内)にとらえたら待機へ戻る
	AddTransition(ST_CHASE, ST_IDLE, 70, isInFov);

	// 歩き・走り中：十分近づいたら待機へ戻る
	AddTransition({ ST_WALK, ST_RUN }, ST_IDLE, 65, isWalkStop);

	// 攻撃中：攻撃行動が完了したら待機へ戻る
	AddTransition(ST_ATTACK, ST_IDLE, 80, isActionFinished);

	// --- 2. 視野角外への対応(旋回) ---
	// 待機中：対象が視野角の外にいるなら旋回へ
	AddTransition(ST_IDLE, ST_CHASE, 60, isOutFov);

	// --- 3. 移動速度(歩き⇔走り)の切り替え ---
	// 歩き中：対象がさらに離れたら走りへ
	AddTransition(ST_WALK, ST_RUN, 40, And(isInFov, isRunStart));
	// 走り中：ある程度近づいたら歩きへ
	AddTransition(ST_RUN, ST_WALK, 40, And(isInFov, isRunStop));

	// --- 4. 待機からの行動開始 ---
	// 待機中：正面かつ遠距離なら走りへ
	AddTransition(ST_IDLE, ST_RUN, 35, And(isInFov, isRunStart));
	// 待機中：正面かつ中距離なら歩きへ
	AddTransition(ST_IDLE, ST_WALK, 30, And(isInFov, isWalkStart));

	// --- 5. 攻撃・死亡の遷移(各ステート実装後に有効化) ---
	 AddTransition({ ST_IDLE, ST_WALK, ST_RUN }, ST_ATTACK, 50, canAttack);
	 AddTransition(ANY_STATE, ST_DIE, 100, isDead);
	(void)canAttack;
	(void)isDead;
	(void)ST_DIE;
}

int BossAl::DecideNextState(const EnemyBlackboard& bb) const
{
	for (const auto& rule : rules_)
	{
		// 遷移元チェック(ANY_STATE または 現在の状態と一致)
		if (rule.fromState != ANY_STATE && rule.fromState != bb.currentState)
		{
			continue;
		}
		// すでに遷移先と同じ状態ならスキップ
		if (rule.toState == bb.currentState)
		{
			continue;
		}
		// 条件式を評価
		if (rule.condition && rule.condition(bb))
		{
			return rule.toState;
		}
	}

	return INVALID_STATE;
}

void BossAl::AddTransition(int fromState, int toState, int priority, const Condition& condition)
{
	rules_.push_back({ fromState, toState, priority, condition });
	std::stable_sort(rules_.begin(), rules_.end(),
		[](const TransitionRule& a, const TransitionRule& b)
		{
			return a.priority > b.priority;
		});
}

void BossAl::AddTransition(
	std::initializer_list<int> fromStates,
	int toState,
	int priority,
	const Condition& condition)
{
	for (int from : fromStates)
	{
		rules_.push_back({ from, toState, priority, condition });
	}
	std::stable_sort(rules_.begin(), rules_.end(),
		[](const TransitionRule& a, const TransitionRule& b)
		{
			return a.priority > b.priority;
		});
}

// --- 条件式の合成ヘルパー ---

BossAl::Condition BossAl::And(const Condition& a, const Condition& b)
{
	return [a, b](const EnemyBlackboard& bb) { return a(bb) && b(bb); };
}

BossAl::Condition BossAl::Or(const Condition& a, const Condition& b)
{
	return [a, b](const EnemyBlackboard& bb) { return a(bb) || b(bb); };
}

BossAl::Condition BossAl::Not(const Condition& a)
{
	return [a](const EnemyBlackboard& bb) { return !a(bb); };
}

// --- 再利用可能な共通条件パーツ ---

BossAl::Condition BossAl::IsInFov(float halfAngleDeg)
{
	return [halfAngleDeg](const EnemyBlackboard& bb)
	{
		return bb.angleToPlayer <= halfAngleDeg;
	};
}

BossAl::Condition BossAl::IsOutOfFov(float halfAngleDeg)
{
	return [halfAngleDeg](const EnemyBlackboard& bb)
	{
		return bb.angleToPlayer > halfAngleDeg;
	};
}

BossAl::Condition BossAl::IsDistLessEq(float dist)
{
	return [dist](const EnemyBlackboard& bb)
	{
		return bb.distance <= dist;
	};
}

BossAl::Condition BossAl::IsDistGreater(float dist)
{
	return [dist](const EnemyBlackboard& bb)
	{
		return bb.distance > dist;
	};
}

BossAl::Condition BossAl::IsDistInRange(float minDist, float maxDist)
{
	return [minDist, maxDist](const EnemyBlackboard& bb)
	{
		return bb.distance >= minDist && bb.distance <= maxDist;
	};
}

BossAl::Condition BossAl::IsHpRateLessEq(float rate)
{
	return [rate](const EnemyBlackboard& bb)
	{
		return bb.hpRate <= rate;
	};
}

BossAl::Condition BossAl::IsDead(void)
{
	return [](const EnemyBlackboard& bb)
	{
		return bb.hpRate <= 0.0f;
	};
}

BossAl::Condition BossAl::IsActionFinished(void)
{
	return [](const EnemyBlackboard& bb)
	{
		return bb.isActionFinished;
	};
}

BossAl::Condition BossAl::IsStateElapsedGreaterEq(float sec)
{
	return [sec](const EnemyBlackboard& bb)
	{
		return bb.stateElapsedTime >= sec;
	};
}

BossAl::Condition BossAl::IsCoolTimeReady(int key)
{
	return [key](const EnemyBlackboard& bb)
	{
		return bb.GetCoolTime(key) <= 0.0f;
	};
}
