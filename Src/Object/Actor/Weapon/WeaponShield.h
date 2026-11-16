#pragma once
#include "WeaponBase.h"

class WeaponShield :public WeaponBase
{
public:
	
	// コンストラクタ
	WeaponShield(const Transform& followTransform, int followFrameId);

	// デストラクタ
	~WeaponShield(void) override;

	// 更新
	void Update(void) override;

	// 衝突判定の設定
	void SetCollider(void) override;

	// 衝突判定のクリア
	void ClearCollider(void) override;

	// 攻撃フラグの設定
	void SetIsAttack(bool isAttack)override;

	// 終了フラグの設定
	void SetIsEnd(bool isEnd) override;

protected:

	// 当たり判定衝突時の更新処理
	void Collision(void) override;

};

