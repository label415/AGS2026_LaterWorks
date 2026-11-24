#pragma once
#include <map>
#include <memory>
#include <functional>
#include "../../StateBase.h"

class EnemyBoss;

class EnemyStateManager
{
public:

	// コンストラクタ
	EnemyStateManager(EnemyBoss& enemyBoss);

	// デストラクタ
	~EnemyStateManager(void);

	// 登録
	void AddState(int stateId, std::unique_ptr<StateBase<EnemyBoss>> state);

	// 条件登録
	void AddConditions(void);

	// 更新
	void Update(void);

	// 解放
	void Release(void);

	// 状態遷移
	void ChangerState(int state);

	// 現在の状態IDを取得
	int GetState(void) const { return state_; }

private:

	// 各状態クラスのインスタンスを保持するマップ
	std::map<int, std::unique_ptr<StateBase<EnemyBoss>>> states_;

	// 状態クラス
	StateBase<EnemyBoss>* currentState_;

	// 敵ボス情報参照
	EnemyBoss& enemyBoss_;

	// 状態
	int state_;

};

