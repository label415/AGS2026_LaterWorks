#include "../../../Manager/ResourceManager.h"
#include "WeaponShield.h"

namespace
{
	constexpr const wchar_t* MODEL_WEAPON_SHIELD_NAME = L"ModelEnemyShield";
}

WeaponShield::WeaponShield(WEAPON_TYPE type, const Transform& followTransform, int followFrameId)
	: WeaponBase(type, followTransform, followFrameId)
{
}

WeaponShield::~WeaponShield(void) {}

void WeaponShield::InitLoad(void)
{
	transform_.SetModel(resMng_.Load(MODEL_WEAPON_SHIELD_NAME).handleId_);
}
