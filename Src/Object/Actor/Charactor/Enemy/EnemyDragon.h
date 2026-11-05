#pragma once
#include "EnemyBase.h"

class EnemyDragon :
    public EnemyBase
{
public:

    // コンストラクタ
    EnemyDragon(void);

    // デストラクタ
    virtual ~EnemyDragon(void) override;

    // 更新
	virtual void Update(void) override;

protected:



};

