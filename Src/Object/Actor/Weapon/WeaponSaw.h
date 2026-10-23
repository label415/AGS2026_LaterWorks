#pragma once
#include <DxLib.h>
#include "../../Common/AnimationController.h"
#include "WeaponBase.h"

class Transform;

class WeaponSaw : public WeaponBase
{
public:
	// コンストラクタ
	WeaponSaw(const Transform& followTransform, int followFrameId);

	// デストラクタ
	~WeaponSaw(void) override{}

	// 更新
	void Update(void) override;

	// コライダーを設定
	void SetCollider(void) override;

	// コライダーをクリア
	void ClearCollider(void) override;

	// 武器変形フラグ設定
	void SetIsTransformation(bool isTransformation) { isTransformation_ = isTransformation; }

	// 武器変形フラグ取得
	const bool GetIsTransformation(void) const { return isTransformation_; }
protected:
	// リソースロード
	void InitLoad(void) override;

	// 大きさ、回転、座標の初期化
	void InitTransform(void) override;

	// 衝突判定の初期化
	void InitCollider(void) override;

	// アニメーションの初期化
	void InitAnimation(void) override;

	// 初期化後の個別処理
	void InitPost(void) override{}

	// GUi更新
	void UpdateGui(void) override;
private:
	// 武器アニメーション番号
	static constexpr int ANIM_WEAPON_ATTACH_NO = 5;
	// 武器アニメーション速度
	static constexpr float ANIM_WEAPON_SPEED = 60.0f;

	// 武器変形最大時間
	static constexpr float END_TRANSFORMATION_TIME = 30.0f;

	// 武器変形前開始時間
	static constexpr float START_TRANSFORMATION_TIME = 120.0f;

	//アニメーションコントローラ
	std::unique_ptr<AnimationController> animationController_;

	// モデルの大きさ
	static constexpr float SCALE = 1.0f;

	// 衝突判定用カプセルの初期値
	static constexpr VECTOR COL_CAPSULE_TOP_LOCAL_POS = { -30.0f, 40.0f, 0.0f };
	static constexpr VECTOR COL_CAPSULE_DOWN_LOCAL_POS = { -30.0f, -40.0f, 0.0f };
	static constexpr float COL_CAPSULE_RADIUS = 20.0f;

	VECTOR colLocalPosU = COL_CAPSULE_TOP_LOCAL_POS;
	VECTOR colLocalPosD = COL_CAPSULE_DOWN_LOCAL_POS;

	// 武器変形フラグ
	bool isTransformation_;

	// 武器変形処理
	void UpdateTransformation(void);
};