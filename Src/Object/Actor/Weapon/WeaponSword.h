#pragma once
#include "WeaponBase.h"

class WeaponSword : public WeaponBase
{
public:
	WeaponSword(WEAPON_TYPE type, const Transform& followTransform, int followFrameId);
	~WeaponSword(void) override;

protected:
	void InitLoad(void) override;
	void InitAnimation(void) override {}
	void InitPost(void) override {}
};
