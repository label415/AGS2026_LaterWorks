#include "../../../Utility/AsoUtility.h"
#include "../../../Utility/ModelFrameUtility.h"
#include "../../../Manager/ResourceManager.h"
#include "../../../Manager/CollisionManager.h"
#include "../../Common/Collider/ColliderCapsule.h"
#include "../../Common/Transform.h"
#include "WeaponBase.h"

namespace
{
	constexpr const wchar_t* JSON_WEAPON_DATA_NAME = L"WeaponData";
}

WeaponBase::WeaponBase(WEAPON_TYPE type, const Transform& followTransform, int followFrameId)
	: weapontype_(type)
	, followTransform_(followTransform)
	, followFrameId_(followFrameId)
	, localPos_(AsoUtility::VECTOR_ZERO)
	, localRot_(AsoUtility::VECTOR_ZERO)
{
	isAlive_ = false;
}

WeaponBase::~WeaponBase(void) {}

Json& WeaponBase::GetData(void) const
{
	auto& root = resMng_.GetJson(JSON_WEAPON_DATA_NAME);
	int targetId = static_cast<int>(weapontype_);
	for (auto& item : root["weapon"])
	{
		if (item.value("id", -1) == targetId)
		{
			return item;
		}
	}
	return root["weapon"][0];
}

void WeaponBase::InitTransform(void)
{
	const auto& trans = GetData()["transform"];
	transform_.scl = VScale(AsoUtility::VECTOR_ONE, trans["scale"].get<float>());
	transform_.quaRot = Quaternion();
	transform_.quaRotLocal = Quaternion();
	transform_.pos = AsoUtility::VECTOR_ZERO;
	localPos_ = trans["localPos"];
	VECTOR rotDeg = trans["localRot"];
	localRot_ = {
		AsoUtility::Deg2RadF(rotDeg.x),
		AsoUtility::Deg2RadF(rotDeg.y),
		AsoUtility::Deg2RadF(rotDeg.z)
	};
}

void WeaponBase::InitCollider(void)
{
	const auto& cap = GetData()["collider"]["capsule"];
	std::vector<ColliderBase::TAG> targetTags = { ColliderBase::TAG::ENEMY };

	auto colliderCapsule = std::make_shared<ColliderCapsule>(
		ColliderBase::TAG::PLAYER_WEPON,
		targetTags,
		&transform_,
		cap["topPos"],
		cap["downPos"],
		cap["radius"].get<float>());
	colliderCapsule->SetIsCollier(false);

	ownColliders_[static_cast<int>(ColliderBase::SHAPE::CAPSULE)].push_back(colliderCapsule);
	CollisionManager::GetInstance().AddCollider(colliderCapsule);
}

void WeaponBase::ApplyJson(void)
{
	InitTransform();

	const auto& cap = GetData()["collider"]["capsule"];
	int capsuleType = static_cast<int>(ColliderBase::SHAPE::CAPSULE);
	if (ownColliders_.count(capsuleType) == 0) return;

	for (auto& col : ownColliders_.at(capsuleType))
	{
		if (auto capsule = std::dynamic_pointer_cast<ColliderCapsule>(col))
		{
			capsule->SetLocalPosTop(cap["topPos"]);
			capsule->SetLocalPosDown(cap["downPos"]);
			capsule->SetRadius(cap["radius"].get<float>());
		}
	}
}

void WeaponBase::Update(void)
{
	ModelFrameUtility::SetFrameWorldMatrix(
		followTransform_, followFrameId_, transform_,
		localPos_, localRot_);
	transform_.quaRot = Quaternion::GetRotation(transform_.matRot);
	transform_.Update();
}

void WeaponBase::SetCollider(void)
{
	isAlive_ = true;
	int capsuleType = static_cast<int>(ColliderBase::SHAPE::CAPSULE);
	for (const auto& vec : ownColliders_[capsuleType])
	{
		vec->SetIsCollier(true);
	}
}

void WeaponBase::ClearCollider(void)
{
	isAlive_ = false;
	int capsuleType = static_cast<int>(ColliderBase::SHAPE::CAPSULE);
	for (const auto& vec : ownColliders_[capsuleType])
	{
		vec->SetIsCollier(false);
	}
}

void WeaponBase::Collision(void)
{
	UpdateHitCollider();
}
