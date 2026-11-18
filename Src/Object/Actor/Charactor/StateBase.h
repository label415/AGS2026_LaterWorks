#pragma once
template <class T>

class StateBase
{
public:

	// デストラクタ
	virtual ~StateBase(void) = default;

	// 開始
	virtual void StartState(T& t) = 0;

	// 更新
	virtual void Update(T& t) = 0;

	// 終了
	virtual void EndState(T& t) = 0;

};