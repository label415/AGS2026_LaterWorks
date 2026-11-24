#pragma once
#include <map>
#include <vector>

// エネミーAIの意思決定判断材料
struct EnemyBlackboard
{
	// プレイヤーとの距離
	float distance = 0.0f;
	// 正面からの角度
	float angleToPlayer = 0.0f;
	// 残りHPの割合
	float hpRate = 1.0f;

	// 現在の状態ID
	int currentState = 0;
	// 現在の状態の経過時間
	float stateElapsedTime = 0.0f;
	// 現在の行動が終了したか
	bool isActionFinished = false;

	// 現在の相手の状態
	int currentStatePlayer = 0;
	// 過去に相手が行った状態の履歴
	std::vector<int> currentStatesPlayer;
	// 行動・スキル別の残りクールタイム
	std::map<int, float> coolTimes;

	// 指定キーの残りクールタイムを取得
	float GetCoolTime(int key) const
	{
		auto it = coolTimes.find(key);
		return (it != coolTimes.end()) ? it->second : 0.0f;
	}

	// 全クールタイムを経過時間分だけ減算する
	void UpdateCoolTimes(float deltaTime)
	{
		for (auto& pair : coolTimes)
		{
			if (pair.second > 0.0f)
			{
				pair.second -= deltaTime;
				if (pair.second < 0.0f) pair.second = 0.0f;
			}
		}
	}
};
