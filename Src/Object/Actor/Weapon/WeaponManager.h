#pragma once
#include <vector>
#include <memory>
#include "WeaponBase.h"

class WeaponManager
{
public:

	// コンストラクタ
	WeaponManager(void);

	// デストラクタ
	~WeaponManager(void);

	// 登録
	void Add(WeaponBase::WEAPON_TYPE weaponType,
		const Transform& followTransform, int followFrameId);

	// 読み込み
	void Load(void);

	// 読み込み後の初期化
	void LoadEnd(void);

	// 更新
	void Update(void);

	// 描画
	void Draw(void);

	// 解放
	void Release(void);

	// 衝突判定の有効化
	void SetCollider(void);

	// 衝突判定のクリア
	void ClearCollider(void);

	// 武器リスト取得
	const std::vector<std::unique_ptr<WeaponBase>>& GetWeapons(void) const { return weapons_; }

private:

	// 武器
	std::vector<std::unique_ptr<WeaponBase>> weapons_;

	// 武器生成
	std::unique_ptr<WeaponBase> CreateWeapon(
		WeaponBase::WEAPON_TYPE type, const Transform& followTransform, int followFrameId);
};
