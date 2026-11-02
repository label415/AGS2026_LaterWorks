#include "../../../../../Utility/AsoUtility.h"
#include "../../../../../Manager/SoundManager.h"
#include "../../../../../Manager/ResourceManager.h"
#include "../../../../../Manager/InputManager.h"
#include "../../../../../Manager/SceneManager.h"
#include "../../../UI/UISt.h"
#include "../Player.h"
#include "PlayerMoveState.h"

PlayerMoveState::PlayerMoveState(void)
	:moveType_(MOVE_TYPE::NONE)
{
}

PlayerMoveState::~PlayerMoveState(void)
{
}

void PlayerMoveState::StartState(Player& player)
{
	// 移動量の初期化
	player.moveSpeed_ = 0.0f;
	player.movePow_ = AsoUtility::VECTOR_ZERO;

	// 状態の初期化
	moveType_ = MOVE_TYPE::NONE;
}

void PlayerMoveState::Update(Player& player)
{
	// 共通化した関数から方向を取得
	VECTOR inputDir = player.GetInputMoveDir();

	// 入力がない場合は、移動状態を終了して待機状態に遷移
	if (AsoUtility::EqualsVZero(inputDir))
	{
		player.ChangeState(Player::STATE::IDLE);
		return;
	}

	// playerの移動方向を更新
	player.moveDir_ = inputDir;

	// ダッシュフラグを取得
	bool canDash = player.GetInputDash() && player.stamina_ <= 0.0f;

	// スタミナが回復中で、かつスタミナが残っている場合はダッシュ可能
	if (canDash) {
		ChangeMoveType(player, MOVE_TYPE::RUN);
	}
	else
	{
		ChangeMoveType(player, MOVE_TYPE::WALK);
	}

	// 移動量の更新
	player.movePow_ = VAdd(player.movePow_, VScale(player.moveDir_, player.moveSpeed_));

	// 移動量の最大値を制限
	float currentSpeed = VSize(player.movePow_);

	// 移動量が100を超える場合は、移動量を正規化して100にスケーリング
	if (currentSpeed > 100.0f)
	{
		player.movePow_ = VScale(VNorm(player.movePow_), 100.0f);
	}

	//移動量の減速処理
	player.movePow_ = AsoUtility::Lerp(player.movePow_, AsoUtility::VECTOR_ZERO, 0.1f);
}

void PlayerMoveState::EndState(Player& player)
{
	// 歩きSE停止
	player.sngMng_.StopSound(SoundManager::SOUND_ID::PLAYER_WAKE);
	player.sngMng_.StopSound(SoundManager::SOUND_ID::PLAYER_RAN);
}

void PlayerMoveState::SetWalk(Player& player)
{
	// 移動速度を設定
	player.moveSpeed_ = player.SPEED_MOVE;

	// アニメーションを再生
	player.animationController_->Play(static_cast<int>(Player::ANIM_TYPE::RUN));

	// 走りSEを止めてから歩きSE再生
	player.sngMng_.StopSound(SoundManager::SOUND_ID::PLAYER_RAN);
	player.sngMng_.PlaySoundId(SoundManager::SOUND_ID::PLAYER_WAKE, true);
}

void PlayerMoveState::SetRun(Player& player)
{
	// 移動速度を設定
	player.moveSpeed_ = player.SPEED_DASH;

	// アニメーションを再生
	player.animationController_->Play(static_cast<int>(Player::ANIM_TYPE::FAST_RUN));

	// 歩きSEを止めてから走りSE再生
	player.sngMng_.StopSound(SoundManager::SOUND_ID::PLAYER_WAKE);
	player.sngMng_.PlaySoundId(SoundManager::SOUND_ID::PLAYER_RAN, true);
}

void PlayerMoveState::ChangeMoveType(Player& player, MOVE_TYPE newType)
{
	// すでに同じ移動タイプの場合は処理しない
	if (moveType_ == newType) return;

	// 移動タイプに応じた処理を実行
	if(newType == MOVE_TYPE::WALK)
	{
		SetWalk(player);
	}
	else if(newType == MOVE_TYPE::RUN)
	{
		SetRun(player);
	}

	// 移動タイプを更新
	moveType_ = newType;
}		