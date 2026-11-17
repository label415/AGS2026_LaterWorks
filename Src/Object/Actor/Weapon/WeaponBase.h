#pragma once
#include <DxLib.h>
#include "../../../Utility/JsonUtility.h"
#include "../ActorBase.h"

class Transform;

class WeaponBase : public ActorBase
{
public:

	// 武器種別
	enum class WEAPON_TYPE
	{
		SAW,
		GAN,
		SHIELD,
		SWORD,
		MAX
	};

	// コンストラクタ
	WeaponBase(WEAPON_TYPE type, const Transform& followTransform, int followFrameId);

	// デストラクタ
	virtual ~WeaponBase(void) override;

	// 更新
	virtual void Update(void) override;

	// 衝突判定の設定
	virtual void SetCollider(void);

	// 衝突判定のクリア
	virtual void ClearCollider(void);

	// 攻撃フラグの設定
	virtual void SetIsAttack(bool isAttack) {}

	// 終了フラグの設定
	virtual void SetIsEnd(bool isEnd) {}

	// JSONの変更をTransform・コライダーに即時反映する
	virtual void ApplyJson(void);

	// 武器種
	WEAPON_TYPE GetWeaponType(void) const { return weapontype_; }

	// WeaponData.json内の該当武器オブジェクトへの直接参照を取得
	Json& GetData(void) const;

protected:

	// 追従先Transform
	const Transform& followTransform_;

	// 追従対象のフレームID
	int followFrameId_;

	// ローカル座標
	VECTOR localPos_;

	// ローカル回転(ラジアン)
	VECTOR localRot_;

	// 武器種
	WEAPON_TYPE weapontype_;

	// 共通のTransform初期化(WeaponData.jsonから直接読み込み)
	void InitTransform(void) override;

	// 共通のカプセルコライダー初期化(WeaponData.jsonから直接読み込み)
	void InitCollider(void) override;

	// 当たり判定衝突時の更新処理
	void Collision(void) override;
};
