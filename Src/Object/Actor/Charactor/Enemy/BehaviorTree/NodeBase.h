#pragma once
class NodeBase
{
public:

	enum class BtStarus
	{
		SUCCESS,
		FAILURE,
		RUNNING
	};

	// デストラクタ
	virtual ~NodeBase(void) = default;

	virtual BtStarus Tick(BtStarus) = 0;

	virtual void Reset(void){}
};

