#pragma once
#include <algorithm>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>
#include "../../StateBase.h"
#include "AiCondition.h"
#include "AiUtility.h"

// 階層型ステートマシン(HFSM) + Condition一括遷移 + Utility選択の統合コントローラ
template <class TOwner, class TBlackboard>
class HfsmController
{
public:
	static constexpr int INVALID_ID = -1;

	// 遷移ルール(条件遷移 / Utility選択遷移の両方を統一的に扱う)
	struct TransitionRule
	{
		// 遷移元ノードID(親グループID または 個別ステートID / INVALID_IDなら全状態)
		int fromNodeId = INVALID_ID;
		// 遷移先ステートID(Utility遷移の場合はINVALID_ID)
		int toStateId = INVALID_ID;
		// 優先度(大きいほど先に評価される)
		int priority = 0;
		// 条件遷移用の条件式
		AiCondition<TBlackboard> condition;
		// Utility遷移用のセレクタ(設定されていればUtility遷移として評価)
		std::shared_ptr<AiUtilitySelector<TBlackboard>> utilitySelector;
		// Utility遷移の最低スコア閾値
		float utilityMinScore = 0.0f;
		// Utility遷移で確率選択を使うか(falseなら最高スコアを選択)
		bool utilityWeighted = false;
		// デバッグ表示用ラベル
		std::string label;
	};

	HfsmController(void)
		: currentStateId_(INVALID_ID)
		, currentState_(nullptr)
		, lastTransitionLabel_("None")
	{
	}

	// 全登録をクリアする
	void Clear(void)
	{
		nodes_.clear();
		states_.clear();
		rules_.clear();
		currentStateId_ = INVALID_ID;
		currentState_ = nullptr;
		lastTransitionLabel_ = "None";
		lastUtilityScores_.clear();
	}

	// 階層グループ(複合ステート)を登録する
	void RegisterGroup(int groupId, const std::string& name, int parentGroupId = INVALID_ID)
	{
		nodes_[groupId] = { groupId, parentGroupId, name, true };
	}

	// 実体ステート(葉ステート)を登録する
	void RegisterState(
		int stateId,
		std::unique_ptr<StateBase<TOwner>> state,
		const std::string& name,
		int parentGroupId = INVALID_ID)
	{
		nodes_[stateId] = { stateId, parentGroupId, name, false };
		states_[stateId] = std::move(state);
	}

	// 条件ベースの遷移ルールを登録する(fromNodeIdに親グループIDを指定すれば配下すべてに適用)
	void AddTransition(
		int fromNodeId,
		int toStateId,
		int priority,
		const AiCondition<TBlackboard>& condition,
		const std::string& label = "")
	{
		TransitionRule rule;
		rule.fromNodeId = fromNodeId;
		rule.toStateId = toStateId;
		rule.priority = priority;
		rule.condition = condition;
		rule.label = label.empty() ? condition.GetName() : label;
		rules_.push_back(std::move(rule));
		SortRules();
	}

	// Utility選択ベースの遷移ルールを登録する
	void AddUtilityTransition(
		int fromNodeId,
		int priority,
		std::shared_ptr<AiUtilitySelector<TBlackboard>> selector,
		float minScore = 0.0f,
		bool weighted = false,
		const std::string& label = "Utility")
	{
		TransitionRule rule;
		rule.fromNodeId = fromNodeId;
		rule.toStateId = INVALID_ID;
		rule.priority = priority;
		rule.utilitySelector = std::move(selector);
		rule.utilityMinScore = minScore;
		rule.utilityWeighted = weighted;
		rule.label = label;
		rules_.push_back(std::move(rule));
		SortRules();
	}

	// 初期ステートを開始する
	void Start(TOwner& owner, int initialStateId)
	{
		auto it = states_.find(initialStateId);
		if (it == states_.end()) return;

		currentStateId_ = initialStateId;
		currentState_ = it->second.get();
		lastTransitionLabel_ = "Start -> " + GetNodeName(initialStateId);
		currentState_->StartState(owner);
	}

	// 遷移ルールを一括評価し、該当すればステート遷移を実行する
	bool EvaluateTransitions(TOwner& owner, const TBlackboard& bb)
	{
		if (currentState_ == nullptr) return false;

		for (const auto& rule : rules_)
		{
			// 遷移元ノードが現在のステート自身、またはその祖先グループに一致するか
			if (!IsInHierarchy(currentStateId_, rule.fromNodeId))
			{
				continue;
			}

			if (rule.utilitySelector)
			{
				// デバッグ用に最新の評価スコアを保存
				lastUtilityScores_ = rule.utilitySelector->EvaluateAll(bb);

				std::string chosenName;
				float chosenScore = 0.0f;
				std::optional<int> nextId = rule.utilityWeighted
					? rule.utilitySelector->SelectWeighted(bb, rule.utilityMinScore, &chosenName, &chosenScore)
					: rule.utilitySelector->SelectBest(bb, rule.utilityMinScore, &chosenName, &chosenScore);

				if (nextId.has_value() && nextId.value() != currentStateId_)
				{
					if (ChangeState(owner, nextId.value(), rule.label + " [" + chosenName + "]"))
					{
						return true;
					}
				}
			}
			else
			{
				if (rule.toStateId == currentStateId_) continue;
				if (rule.condition.Evaluate(bb))
				{
					if (ChangeState(owner, rule.toStateId, rule.label))
					{
						return true;
					}
				}
			}
		}

		return false;
	}

	// ステートを強制遷移する
	bool ChangeState(TOwner& owner, int nextStateId, const std::string& reason = "Manual")
	{
		if (currentStateId_ == nextStateId) return false;

		auto it = states_.find(nextStateId);
		if (it == states_.end()) return false;

		if (currentState_ != nullptr)
		{
			currentState_->EndState(owner);
		}

		std::string prevName = GetNodeName(currentStateId_);
		currentStateId_ = nextStateId;
		currentState_ = it->second.get();
		lastTransitionLabel_ = prevName + " -> " + GetNodeName(nextStateId) + " (" + reason + ")";
		currentState_->StartState(owner);
		return true;
	}

	// 現在ステートの更新
	void Update(TOwner& owner)
	{
		if (currentState_ != nullptr)
		{
			currentState_->Update(owner);
		}
	}

	// 現在ステートの更新後処理(アニメーション更新後)
	void UpdatePost(TOwner& owner)
	{
		if (currentState_ != nullptr)
		{
			currentState_->UpdatePost(owner);
		}
	}

	// 現在のステートIDを取得
	int GetCurrentStateId(void) const
	{
		return currentStateId_;
	}

	// 現在のステート名を取得
	std::string GetCurrentStateName(void) const
	{
		return GetNodeName(currentStateId_);
	}

	// 現在の階層パス文字列を取得(例: "ALIVE / NEUTRAL / IDLE")
	std::string GetCurrentHierarchyPath(void) const
	{
		std::vector<std::string> names;
		int id = currentStateId_;
		int guard = 0;
		while (id != INVALID_ID && guard++ < 16)
		{
			auto it = nodes_.find(id);
			if (it == nodes_.end()) break;
			names.push_back(it->second.name);
			id = it->second.parentId;
		}
		std::string path;
		for (auto it = names.rbegin(); it != names.rend(); ++it)
		{
			if (!path.empty()) path += " / ";
			path += *it;
		}
		return path.empty() ? "None" : path;
	}

	// 直前の遷移理由ラベルを取得
	const std::string& GetLastTransitionLabel(void) const
	{
		return lastTransitionLabel_;
	}

	// 直前のUtilityスコア一覧を取得
	const std::vector<AiUtilityScoreInfo>& GetLastUtilityScores(void) const
	{
		return lastUtilityScores_;
	}

	// 指定ステートがtargetNodeId(グループまたはステート)に属しているか判定
	bool IsInHierarchy(int stateId, int targetNodeId) const
	{
		if (targetNodeId == INVALID_ID) return true;

		int id = stateId;
		int guard = 0;
		while (id != INVALID_ID && guard++ < 16)
		{
			if (id == targetNodeId) return true;
			auto it = nodes_.find(id);
			if (it == nodes_.end()) break;
			id = it->second.parentId;
		}
		return false;
	}

private:
	struct NodeInfo
	{
		int id = INVALID_ID;
		int parentId = INVALID_ID;
		std::string name;
		bool isGroup = false;
	};

	std::map<int, NodeInfo> nodes_;
	std::map<int, std::unique_ptr<StateBase<TOwner>>> states_;
	std::vector<TransitionRule> rules_;

	int currentStateId_;
	StateBase<TOwner>* currentState_;
	std::string lastTransitionLabel_;
	std::vector<AiUtilityScoreInfo> lastUtilityScores_;

	void SortRules(void)
	{
		std::stable_sort(rules_.begin(), rules_.end(),
			[](const TransitionRule& a, const TransitionRule& b)
			{
				return a.priority > b.priority;
			});
	}

	std::string GetNodeName(int id) const
	{
		auto it = nodes_.find(id);
		return (it != nodes_.end()) ? it->second.name : "Unknown";
	}
};
