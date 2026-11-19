#pragma once
#include <functional>
#include <string>
#include <utility>
#include "EnemyBlackboard.h"

// 再利用・論理合成(&& / || / !)可能な条件オブジェクト
template <class TBlackboard>
class AiCondition
{
public:
	using Func = std::function<bool(const TBlackboard&)>;

	AiCondition(void)
		: func_([](const TBlackboard&) { return false; })
		, name_("Never")
	{
	}

	AiCondition(Func func, std::string name = "")
		: func_(std::move(func))
		, name_(std::move(name))
	{
	}

	// 条件を評価する
	bool Evaluate(const TBlackboard& bb) const
	{
		return func_ ? func_(bb) : false;
	}

	// デバッグ表示用の条件名を取得する
	const std::string& GetName(void) const
	{
		return name_;
	}

	// AND合成
	AiCondition operator&&(const AiCondition& rhs) const
	{
		Func lhsFunc = func_;
		Func rhsFunc = rhs.func_;
		return AiCondition(
			[lhsFunc, rhsFunc](const TBlackboard& bb)
			{
				return lhsFunc(bb) && rhsFunc(bb);
			},
			"(" + name_ + " && " + rhs.name_ + ")");
	}

	// OR合成
	AiCondition operator||(const AiCondition& rhs) const
	{
		Func lhsFunc = func_;
		Func rhsFunc = rhs.func_;
		return AiCondition(
			[lhsFunc, rhsFunc](const TBlackboard& bb)
			{
				return lhsFunc(bb) || rhsFunc(bb);
			},
			"(" + name_ + " || " + rhs.name_ + ")");
	}

	// NOT反転
	AiCondition operator!(void) const
	{
		Func targetFunc = func_;
		return AiCondition(
			[targetFunc](const TBlackboard& bb)
			{
				return !targetFunc(bb);
			},
			"!" + name_);
	}

private:
	Func func_;
	std::string name_;
};

using EnemyCondition = AiCondition<EnemyBlackboard>;

// 状態クラス・他エネミーでも使い回せる共通条件パーツ集
namespace EnemyConditions
{
	inline EnemyCondition Always(void)
	{
		return EnemyCondition([](const EnemyBlackboard&) { return true; }, "Always");
	}

	inline EnemyCondition Never(void)
	{
		return EnemyCondition([](const EnemyBlackboard&) { return false; }, "Never");
	}

	inline EnemyCondition DistanceLessEq(float dist)
	{
		return EnemyCondition(
			[dist](const EnemyBlackboard& bb) { return bb.distance <= dist; },
			"Dist<=" + std::to_string(static_cast<int>(dist)));
	}

	inline EnemyCondition DistanceGreater(float dist)
	{
		return EnemyCondition(
			[dist](const EnemyBlackboard& bb) { return bb.distance > dist; },
			"Dist>" + std::to_string(static_cast<int>(dist)));
	}

	inline EnemyCondition DistanceInRange(float minDist, float maxDist)
	{
		return EnemyCondition(
			[minDist, maxDist](const EnemyBlackboard& bb)
			{
				return bb.distance >= minDist && bb.distance <= maxDist;
			},
			"Dist[" + std::to_string(static_cast<int>(minDist)) + "," +
			std::to_string(static_cast<int>(maxDist)) + "]");
	}

	inline EnemyCondition InFov(float halfAngleDeg)
	{
		return EnemyCondition(
			[halfAngleDeg](const EnemyBlackboard& bb) { return bb.angleToPlayer <= halfAngleDeg; },
			"InFov(" + std::to_string(static_cast<int>(halfAngleDeg)) + ")");
	}

	inline EnemyCondition OutOfFov(float halfAngleDeg)
	{
		return EnemyCondition(
			[halfAngleDeg](const EnemyBlackboard& bb) { return bb.angleToPlayer > halfAngleDeg; },
			"OutOfFov(" + std::to_string(static_cast<int>(halfAngleDeg)) + ")");
	}

	inline EnemyCondition HpRateLessEq(float rate)
	{
		return EnemyCondition(
			[rate](const EnemyBlackboard& bb) { return bb.hpRate <= rate; },
			"HpRate<=" + std::to_string(rate));
	}

	inline EnemyCondition HpRateGreater(float rate)
	{
		return EnemyCondition(
			[rate](const EnemyBlackboard& bb) { return bb.hpRate > rate; },
			"HpRate>" + std::to_string(rate));
	}

	inline EnemyCondition IsDead(void)
	{
		return EnemyCondition(
			[](const EnemyBlackboard& bb) { return bb.hpRate <= 0.0f; },
			"IsDead");
	}

	inline EnemyCondition StateElapsedGreaterEq(float sec)
	{
		return EnemyCondition(
			[sec](const EnemyBlackboard& bb) { return bb.stateElapsedTime >= sec; },
			"Elapsed>=" + std::to_string(sec));
	}

	inline EnemyCondition IsActionFinished(void)
	{
		return EnemyCondition(
			[](const EnemyBlackboard& bb) { return bb.isActionFinished; },
			"ActionFinished");
	}

	inline EnemyCondition CoolTimeReady(int key)
	{
		return EnemyCondition(
			[key](const EnemyBlackboard& bb) { return bb.GetCoolTime(key) <= 0.0f; },
			"CoolTimeReady(" + std::to_string(key) + ")");
	}

	inline EnemyCondition PlayerInState(int playerState)
	{
		return EnemyCondition(
			[playerState](const EnemyBlackboard& bb) { return bb.currentStatePlayer == playerState; },
			"PlayerState==" + std::to_string(playerState));
	}
}
