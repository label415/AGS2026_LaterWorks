#include "../../../Manager/ResourceManager.h"
#include "WeaponSword.h"

namespace
{
	constexpr const wchar_t* MODEL_WEAPON_SWORD_NAME = L"ModelEnemySword";
}

WeaponSword::WeaponSword(WEAPON_TYPE type, const Transform& followTransform, int followFrameId)
	: WeaponBase(type, followTransform, followFrameId)
{
}

WeaponSword::~WeaponSword(void) {}

void WeaponSword::InitLoad(void)
{
	transform_.SetModel(resMng_.Load(MODEL_WEAPON_SWORD_NAME).handleId_);
}
