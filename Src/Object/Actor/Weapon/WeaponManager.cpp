#include "../../../Manager/ResourceManager.h"
#include "../../../Manager/EditorManager.h"
#include "WeaponBase.h"
#include "WeaponGan.h"
#include "WeaponSaw.h"
#include "WeaponShield.h"
#include "WeaponSword.h"
#include "WeaponManager.h"

namespace
{
	constexpr const wchar_t* JSON_WEAPON_DATA_NAME = L"WeaponData";
}

WeaponManager::WeaponManager(void)
{
}

WeaponManager::~WeaponManager(void)
{
}

void WeaponManager::Add(
	WeaponBase::WEAPON_TYPE weaponType,
	const Transform& followTransform, int followFrameId)
{
	std::unique_ptr<WeaponBase> newWeapon =
		CreateWeapon(weaponType, followTransform, followFrameId);
	if (!newWeapon) return;
	weapons_.push_back(std::move(newWeapon));
}

void WeaponManager::Load(void)
{
	for (const auto& weapon : weapons_)
	{
		if (weapon) weapon->Load();
	}
}

void WeaponManager::LoadEnd(void)
{
	for (const auto& weapon : weapons_)
	{
		if (weapon) weapon->LoadEnd();
	}

#ifdef _DEBUG
	EditorManager::GetInstance().RegisterObject({
		"Weapon",
		JSON_WEAPON_DATA_NAME,
		[this]() {
			for (const auto& weapon : weapons_)
			{
				if (weapon) weapon->ApplyJson();
			}
		},
		nullptr
	});
#endif
}

void WeaponManager::Update(void)
{
	for (const auto& weapon : weapons_)
	{
		if (weapon) weapon->Update();
	}
}

void WeaponManager::Draw(void)
{
	for (const auto& weapon : weapons_)
	{
		if (weapon) weapon->Draw();
	}
}

void WeaponManager::Release(void)
{
	for (const auto& weapon : weapons_)
	{
		if (weapon) weapon->Release();
	}
	weapons_.clear();
}

void WeaponManager::SetCollider(void)
{
	for (const auto& weapon : weapons_)
	{
		if (weapon) weapon->SetCollider();
	}
}

void WeaponManager::ClearCollider(void)
{
	for (const auto& weapon : weapons_)
	{
		if (weapon) weapon->ClearCollider();
	}
}

std::unique_ptr<WeaponBase> WeaponManager::CreateWeapon(
	WeaponBase::WEAPON_TYPE type, const Transform& followTransform, int followFrameId)
{
	switch (type)
	{
	case WeaponBase::WEAPON_TYPE::SAW:
		return std::make_unique<WeaponSaw>(type, followTransform, followFrameId);
	case WeaponBase::WEAPON_TYPE::SHIELD:
		return std::make_unique<WeaponShield>(type, followTransform, followFrameId);
	case WeaponBase::WEAPON_TYPE::SWORD:
		return std::make_unique<WeaponSword>(type, followTransform, followFrameId);
	case WeaponBase::WEAPON_TYPE::GAN:
	default:
		return nullptr;
	}
}
