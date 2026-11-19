#pragma once
#include <algorithm>
#include <functional>
#include <optional>
#include <random>
#include <string>
#include <utility>
#include <vector>
#include "AiCondition.h"
#include "EnemyBlackboard.h"

// Utility AIの考慮関数(Consideration): Blackboardから0.0～1.0のスコアを算出する部品
template <class TBlackboard>
class AiConsideration
{
public:
	using Func = std::function<float(const TBlackboard&)>;

	AiConsideration(Func func, std::string name = "")
		: func_(std::move(func))
		, name_(std::move(name))
	{
	}

	float Score(const TBlackboard& bb) const
	{
		if (!func_) return 0.0f;
		return std::clamp(func_(bb), 0.0f, 1.0f);
	}

	const std::string& GetName(void) const
	{
		return name_;
	}

private:
	Func func_;
	std::string name_;
};

// Utility AIの候補行動(状態IDまたはスキルID + 基本重み + 複数の考慮関数)
template <class TBlackboard>
struct AiUtilityOption
{
	int targetId = -1;
	float weight = 1.0f;
	std::string name;
	std::vector<AiConsideration<TBlackboard>> considerations;

	// 全考慮関数を掛け合わせて最終スコアを算出する(1つでも0なら即0で足切り)
	float Evaluate(const TBlackboard& bb) const
	{
		if (weight <= 0.0f) return 0.0f;
		float score = weight;
		for (const auto& c : considerations)
		{
			float s = c.Score(bb);
			if (s <= 0.0f) return 0.0f;
			score *= s;
		}
		return score;
	}
};

// デバッグ表示用のスコア結果
struct AiUtilityScoreInfo
{
	int targetId = -1;
	std::string name;
	float score = 0.0f;
};

// Utility AIセレクタ: 複数の候補から最高スコアまたは重み付き確率で行動を選ぶ
template <class TBlackboard>
class AiUtilitySelector
{
public:
	using Option = AiUtilityOption<TBlackboard>;

	void Clear(void)
	{
		options_.clear();
	}

	void AddOption(const Option& option)
	{
		options_.push_back(option);
	}

	void AddOption(
		int targetId,
		float weight,
		const std::string& name,
		std::vector<AiConsideration<TBlackboard>> considerations)
	{
		options_.push_back({ targetId, weight, name, std::move(considerations) });
	}

	// 最もスコアが高い候補を選択する(minScore以下の候補は無視)
	std::optional<int> SelectBest(
		const TBlackboard& bb,
		float minScore = 0.0f,
		std::string* outName = nullptr,
		float* outScore = nullptr) const
	{
		int bestId = -1;
		float bestScore = minScore;
		std::string bestName;

		for (const auto& opt : options_)
		{
			float score = opt.Evaluate(bb);
			if (score > bestScore)
			{
				bestScore = score;
				bestId = opt.targetId;
				bestName = opt.name;
			}
		}

		if (bestId < 0)
		{
			return std::nullopt;
		}

		if (outName) *outName = bestName;
		if (outScore) *outScore = bestScore;
		return bestId;
	}

	// スコア比例の確率で候補を選択する(行動に自然な揺らぎを持たせたい場合)
	std::optional<int> SelectWeighted(
		const TBlackboard& bb,
		float minScore = 0.0f,
		std::string* outName = nullptr,
		float* outScore = nullptr) const
	{
		std::vector<AiUtilityScoreInfo> valid;
		float total = 0.0f;

		for (const auto& opt : options_)
		{
			float score = opt.Evaluate(bb);
			if (score > minScore)
			{
				valid.push_back({ opt.targetId, opt.name, score });
				total += score;
			}
		}

		if (valid.empty() || total <= 0.0f)
		{
			return std::nullopt;
		}

		static std::mt19937 rng(std::random_device{}());
		std::uniform_real_distribution<float> dist(0.0f, total);
		float r = dist(rng);

		float accum = 0.0f;
		for (const auto& item : valid)
		{
			accum += item.score;
			if (r <= accum)
			{
				if (outName) *outName = item.name;
				if (outScore) *outScore = item.score;
				return item.targetId;
			}
		}

		const auto& last = valid.back();
		if (outName) *outName = last.name;
		if (outScore) *outScore = last.score;
		return last.targetId;
	}

	// デバッグ表示用に全候補の現在スコアを返す
	std::vector<AiUtilityScoreInfo> EvaluateAll(const TBlackboard& bb) const
	{
		std::vector<AiUtilityScoreInfo> results;
		results.reserve(options_.size());
		for (const auto& opt : options_)
		{
			results.push_back({ opt.targetId, opt.name, opt.Evaluate(bb) });
		}
		return results;
	}

private:
	std::vector<Option> options_;
};

using EnemyConsideration = AiConsideration<EnemyBlackboard>;
using EnemyUtilityOption = AiUtilityOption<EnemyBlackboard>;
using EnemyUtilitySelector = AiUtilitySelector<EnemyBlackboard>;

// 再利用可能な共通考慮関数(Consideration)パーツ集
namespace EnemyConsiderations
{
	// 既存のEnemyConditionをそのまま考慮関数(0.0 or 1.0)として再利用する
	inline EnemyConsideration FromCondition(
		const EnemyCondition& cond, float trueScore = 1.0f, float falseScore = 0.0f)
	{
		return EnemyConsideration(
			[cond, trueScore, falseScore](const EnemyBlackboard& bb)
			{
				return cond.Evaluate(bb) ? trueScore : falseScore;
			},
			cond.GetName());
	}

	// 距離に応じた線形スコア(invert=false: 遠いほど高い / invert=true: 近いほど高い)
	inline EnemyConsideration DistanceLinear(float minDist, float maxDist, bool invert = false)
	{
		return EnemyConsideration(
			[minDist, maxDist, invert](const EnemyBlackboard& bb)
			{
				if (maxDist <= minDist) return 1.0f;
				float t = std::clamp((bb.distance - minDist) / (maxDist - minDist), 0.0f, 1.0f);
				return invert ? (1.0f - t) : t;
			},
			"DistLinear");
	}

	// HP割合に応じた線形スコア(invert=true: HPが減るほど高スコア)
	inline EnemyConsideration HpRateLinear(float minRate, float maxRate, bool invert = true)
	{
		return EnemyConsideration(
			[minRate, maxRate, invert](const EnemyBlackboard& bb)
			{
				if (maxRate <= minRate) return 1.0f;
				float t = std::clamp((bb.hpRate - minRate) / (maxRate - minRate), 0.0f, 1.0f);
				return invert ? (1.0f - t) : t;
			},
			"HpRateLinear");
	}

	// 正面からの角度に応じたスコア(正面に近いほど1.0、180度で0.0)
	inline EnemyConsideration FacingLinear(bool invert = false)
	{
		return EnemyConsideration(
			[invert](const EnemyBlackboard& bb)
			{
				float t = std::clamp(bb.angleToPlayer / 180.0f, 0.0f, 1.0f);
				return invert ? t : (1.0f - t);
			},
			"FacingLinear");
	}
}
