#include "../../../Manager/ResourceManager.h"
#include "WeaponSaw.h"

namespace
{
	constexpr const wchar_t* MODEL_WEAPON_SAW_NAME = L"ModelPlayerSaw";
}

WeaponSaw::WeaponSaw(WEAPON_TYPE type, const Transform& followTransform, int followFrameId)
	: WeaponBase(type, followTransform, followFrameId)
	, isTransformation_(false)
{
}

void WeaponSaw::Update(void)
{
	WeaponBase::Update();
	animationController_->Update();
	UpdateTransformation();
}

void WeaponSaw::InitLoad(void)
{
	transform_.SetModel(resMng_.Load(MODEL_WEAPON_SAW_NAME).handleId_);
	animationController_ = std::make_unique<AnimationController>(transform_.modelId);
}

void WeaponSaw::InitAnimation(void)
{
	const auto& anim = GetData()["animation"];
	animationController_->Add(anim["attachNo"], anim["speed"], transform_.modelId);
}

void WeaponSaw::ApplyJson(void)
{
	WeaponBase::ApplyJson();
	if (animationController_ && GetData().contains("animation"))
	{
		const auto& anim = GetData()["animation"];
		animationController_->SetSpeed(anim["attachNo"], anim["speed"]);
	}
}

void WeaponSaw::UpdateTransformation(void)
{
	if (!isTransformation_) return;

	const auto& anim = GetData()["animation"];
	animationController_->Play(anim["attachNo"], false);

	if (animationController_->GetPlayAnim().step >= anim["endTransformationTime"].get<float>())
	{
		animationController_->SetIsStopFlager(true);
		animationController_->SetStateTime(anim["startTransformationTime"].get<float>());
		isTransformation_ = false;
	}
}
