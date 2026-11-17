#pragma once
#include "WeaponBase.h"

class WeaponShield : public WeaponBase
{
public:
	WeaponShield(WEAPON_TYPE type, const Transform& followTransform, int followFrameId);
	~WeaponShield(void) override;

protected:
	void InitLoad(void) override;
	void InitAnimation(void) override {}
	void InitPost(void) override {}
};
