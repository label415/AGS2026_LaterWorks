#include "../../../../Manager/ResourceManager.h"
#include "../../../../Manager/SceneManager.h"
#include "../../../../Manager/CollisionManager.h"
#include "../../../../Manager/Camera.h"
#include "../../../../Manager/InputManager.h"
#include "../../../../Manager/SoundManager.h"
#include "../../../Common/Collider/ColliderLine.h"
#include "../../../Common/Collider/ColliderCapsule.h"
#include "EnemyKnight.h"

// リソース名
namespace
{
	// エネミーモデル
	constexpr const wchar_t* MODEL_ENEMY_BOSS_NAME = L"ModelEnemyBoss";
	// エネミーデータ
	constexpr const wchar_t* JSON_ENEMY_DATA_NAME = L"EnemyData";
}

EnemyKnight::EnemyKnight(void)
	: EnemyBase()
{
}

EnemyKnight::~EnemyKnight(void)
{
}

void EnemyKnight::InitLoad(void)
{
	//プレイヤーモデル
	transform_.SetModel(resMng_.LoadModelDuplicate(
		MODEL_ENEMY_BOSS_NAME));
}

void EnemyKnight::InitTransform(void)
{
	// Transformの初期化
	const auto& playerData =
		resMng_.Load(JSON_ENEMY_DATA_NAME).jsonData_;
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

void EnemyKnight::InitCollider(void)
{
	// コライダーデータの読み込み
	const auto& enemyData =
		resMng_.Load(JSON_ENEMY_DATA_NAME).jsonData_;
	const auto& colData = enemyData["collider"];

	// 線分コライダ
	const auto& lineData = colData["line"];
	VECTOR lineStartPos = {
		lineData["startPos"]["x"],
		lineData["startPos"]["y"],
		lineData["startPos"]["z"]
	};
	VECTOR lineEndPos = {
		lineData["endPos"]["x"],
		lineData["endPos"]["y"],
		lineData["endPos"]["z"]
	};

	std::vector<ColliderBase::TAG> lineTags = { ColliderBase::TAG::STAGE };
	auto colLine = std::make_shared<ColliderLine>(
		ColliderBase::TAG::GROUND,
		lineTags,
		&transform_,
		lineStartPos,
		lineEndPos);

	// マネージャーへ登録
	ownColliders_[static_cast<int>(ColliderBase::SHAPE::LINE)].push_back(colLine);
	colMng_.AddCollider(colLine);

	// カプセルコライダ
	const auto& capsuleData = colData["capsule"];
	VECTOR capTopPos = {
		capsuleData["topPos"]["x"],
		capsuleData["topPos"]["y"],
		capsuleData["topPos"]["z"]
	};
	VECTOR capDownPos = {
		capsuleData["downPos"]["x"],
		capsuleData["downPos"]["y"],
		capsuleData["downPos"]["z"]
	};
	float capRadius = capsuleData["radius"];

	std::vector<ColliderBase::TAG> capsuleTags = {
		ColliderBase::TAG::PLAYER,
		ColliderBase::TAG::PLAYER_WEPON,
		ColliderBase::TAG::STAGE
	};

	auto colCapsule = std::make_shared<ColliderCapsule>(
		ColliderBase::TAG::ENEMY,
		capsuleTags,
		&transform_,
		capTopPos,
		capDownPos,
		capRadius);

	// マネージャーへ登録
	ownColliders_[static_cast<int>(ColliderBase::SHAPE::CAPSULE)].push_back(colCapsule);
	colMng_.AddCollider(colCapsule);
}

void EnemyKnight::InitAnimation(void)
{
	// アニメーション
	animationController_ = std::make_unique<AnimationController>(transform_.modelId);

	// アニメーション速度の読み込み
	const auto& playerData =
		resMng_.Load(JSON_ENEMY_DATA_NAME).jsonData_;
	const auto& anim = playerData["animationSpeed"];

	// アニメーション登録
	int type = -1;
	type = static_cast<int>(ANIM_TYPE::IDLE);
	animationController_->AddInFbx(type, anim["idle"].get<float>(), type);
	type = static_cast<int>(ANIM_TYPE::WALK);
	animationController_->AddInFbx(type, anim["walk"].get<float>(), type);
	type = static_cast<int>(ANIM_TYPE::RUN);
	animationController_->AddInFbx(type, anim["run"].get<float>(), type);
	type = static_cast<int>(ANIM_TYPE::MELEE_ATTACK);
	animationController_->AddInFbx(type, anim["sweepAttack"].get<float>(), type);

	// アニメーション再生
	animationController_->Play(static_cast<int>(ANIM_TYPE::IDLE));
}

void EnemyKnight::InitPost(void)
{
}

void EnemyKnight::UpdateProcess(void)
{
}

void EnemyKnight::UpdateProcessPost(void)
{
}

void EnemyKnight::UpdateHitCollider(void)
{
}