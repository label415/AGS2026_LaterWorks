#include "../../../Libs/ImGui/imgui.h"
#include "../../../Utility/AsoUtility.h"
#include "../../../Manager/ResourceManager.h"
#include "../../../Manager/CollisionManager.h"
#include "../../Common/Collider/ColliderCapsule.h"
#include "../ActorBase.h"
#include "WeaponSaw.h"

WeaponSaw::WeaponSaw(const Transform& followTransform, int followFrameId)
	: WeaponBase(WeaponBase::WEAPON_TYPE::BLADE, followTransform, followFrameId)
	, isTransformation_(false)
{
	// 生存フラグを無効にする
	isAlive_ = false;
}

void WeaponSaw::Update(void)
{
	// 基底クラスを更新
	WeaponBase::Update();

	// アニメーション再生
	animationController_->Update();

	// 武器の変形処理
	UpdateTransformation();

#ifdef _DEBUG
	UpdateGui();
#endif
}

void WeaponSaw::InitLoad(void)
{
	// モデルのロード
	transform_.SetModel(
		resMng_.Load(ResourceManager::SRC::MODEL_WEAPON_SAW).handleId_);

	animationController_ =
		std::make_unique<AnimationController>(transform_.modelId);
}

void WeaponSaw::InitTransform(void)
{
	// モデルの大きさ、回転、座標の初期化
	transform_.scl = VScale(AsoUtility::VECTOR_ONE, SCALE);
	transform_.quaRot = Quaternion();
	transform_.quaRotLocal = Quaternion();
	transform_.pos = AsoUtility::VECTOR_ZERO;
	localPos_ = { 0.0f, 0.0f, 0.0f };
	localRot_ = {
	AsoUtility::Deg2RadF(0.0f),
	AsoUtility::Deg2RadF(0.0f),
	AsoUtility::Deg2RadF(90.0f)
	};
}

void WeaponSaw::InitCollider(void)
{
	// 攻撃対象タグ（敵本体など）
	std::vector<ColliderBase::TAG> targetTags = {
		ColliderBase::TAG::ENEMY
	};

	// 武器コライダーを初期化時に生成
	auto colliderCapsule = std::make_shared<ColliderCapsule>(
		ColliderBase::TAG::PLAYER_WEPON,
		targetTags,
		&transform_,
		COL_CAPSULE_TOP_LOCAL_POS,
		COL_CAPSULE_DOWN_LOCAL_POS,
		COL_CAPSULE_RADIUS
	);

	// 初期状態は当たり判定を無効化
	colliderCapsule->SetIsCollier(false);

	ownColliders_
		[static_cast<int>(ColliderBase::SHAPE::CAPSULE)].push_back(colliderCapsule);
	CollisionManager::GetInstance().AddCollider(colliderCapsule);
}

void WeaponSaw::InitAnimation(void)
{
	// アニメーション登録
	animationController_->Add(
		ANIM_WEAPON_ATTACH_NO
		, ANIM_WEAPON_SPEED
		, transform_.modelId);
}

void WeaponSaw::UpdateGui(void)
{
	ImGui::Begin("WeaponSaw");

	ImGui::Text("position");
	ImGui::InputFloat3("ColLocalPosU", &colLocalPosU.x);
	ImGui::InputFloat3("ColLocalPosD", &colLocalPosD.x);

	ImGui::End();

	int capsuleType = static_cast<int>(ColliderBase::SHAPE::CAPSULE);
	for (const auto& vec : ownColliders_.at(capsuleType))
	{
		auto capsuleCollider = std::dynamic_pointer_cast<ColliderCapsule>(vec);
		// 生成時の初期座標・初期サイズに戻す
		capsuleCollider->SetLocalPosTop(colLocalPosU);
		capsuleCollider->SetLocalPosDown(colLocalPosD);
	}
}

void WeaponSaw::UpdateTransformation(void)
{
	if (!isTransformation_)return;

	// アニメション再生
	animationController_->Play(ANIM_WEAPON_ATTACH_NO, false);

	if (animationController_->GetPlayAnim().step
		>= END_TRANSFORMATION_TIME)
	{
		animationController_->SetIsStopFlager(true);
		animationController_->SetStateTime(START_TRANSFORMATION_TIME);
		isTransformation_ = false;
	}
}

void WeaponSaw::SetCollider(void)
{
	isAlive_ = true;
	int capsuleType = static_cast<int>(ColliderBase::SHAPE::CAPSULE);
	for (const auto& vec : ownColliders_.at(capsuleType))
	{
		vec->SetIsCollier(true);
	}
}

void WeaponSaw::ClearCollider(void)
{
	isAlive_ = false;
	int capsuleType = static_cast<int>(ColliderBase::SHAPE::CAPSULE);
	for (const auto& vec : ownColliders_.at(capsuleType))
	{
		auto capsuleCollider = std::dynamic_pointer_cast<ColliderCapsule>(vec);
		// 当たり判定をオフにする
		capsuleCollider->SetIsCollier(false);
	}
}
