#include "../../../../Manager/ResourceManager.h"
#include "../../../../Manager/SceneManager.h"
#include "../../../../Manager/CollisionManager.h"
#include "../../../../Manager/Camera.h"
#include "../../../../Manager/InputManager.h"
#include "../../../../Manager/SoundManager.h"
#include "EnemyDragon.h"

EnemyDragon::EnemyDragon(void)
	: EnemyBase()
{
}

EnemyDragon::~EnemyDragon(void)
{
}

void EnemyDragon::Update(void)
{
}

void EnemyDragon::InitLoad(void)
{
	//プレイヤーモデル
	transform_.SetModel(resMng_.LoadModelDuplicate(
		ResourceManager::SRC::MODEL_ENEMY_DRAGON));
}

void EnemyDragon::InitTransform(void)
{
	// Transformの初期化
	const auto& playerData =
		resMng_.Load(ResourceManager::SRC::JSON_ENEMY).jsonData_;
	const auto& playerTrans = playerData["Transform"];

	// 座標の初期化
	const auto& playerPos = playerTrans["Position"];
	transform_.pos = { playerPos["x"], playerPos["y"], playerPos["z"] };

	// 大きさの初期化
	const auto& playerScale = playerTrans["Scale"];
	transform_.scl = { playerScale["x"], playerScale["y"], playerScale["z"] };

	// 回転の初期化
	transform_.quaRot = Quaternion::Identity();
	transform_.quaRotLocal = Quaternion::Identity();
	const auto& playerRotY = playerTrans["Rotation"]["y"];
	transform_.quaRotLocal =
		Quaternion::Mult(transform_.quaRotLocal,
			Quaternion::AngleAxis(AsoUtility::Deg2RadF(playerRotY), AsoUtility::AXIS_Y));

	// Transformの更新
	transform_.Update();
}

void EnemyDragon::InitCollider(void)
{
}

void EnemyDragon::InitAnimation(void)
{
	// アニメーション
	animationController_ = std::make_unique<AnimationController>(transform_.modelId);

	// アニメーション速度の読み込み
	const auto& playerData =
		resMng_.Load(ResourceManager::SRC::JSON_ENEMY).jsonData_;
	const auto& anim = playerData["animationSpeed"];

	// アニメーション登録
	int type = -1;
	type = static_cast<int>(ANIM_TYPE::IDLE);
	animationController_->AddInFbx(type, anim["idle"].get<float>(), type);
	type = static_cast<int>(ANIM_TYPE::IDLE);
	animationController_->AddInFbx(type, anim["idle"].get<float>(), type);
}

void EnemyDragon::InitPost(void)
{
}

void EnemyDragon::UpdateProcess(void)
{
}

void EnemyDragon::UpdateProcessPost(void)
{
}

void EnemyDragon::UpdateHitCollider(void)
{
}
