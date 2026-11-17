#pragma once
#include <DxLib.h>
#include "../../Common/AnimationController.h"
#include "WeaponBase.h"

class Transform;

class WeaponSaw : public WeaponBase
{
public:
	WeaponSaw(WEAPON_TYPE type, const Transform& followTransform, int followFrameId);
	~WeaponSaw(void) override {}

	void Update(void) override;
	void ApplyJson(void) override;

	void SetIsTransformation(bool isTransformation) { isTransformation_ = isTransformation; }
	const bool GetIsTransformation(void) const { return isTransformation_; }

protected:
	void InitLoad(void) override;
	void InitAnimation(void) override;
	void InitPost(void) override {}

private:
	std::unique_ptr<AnimationController> animationController_;
	bool isTransformation_;

	void UpdateTransformation(void);
};
