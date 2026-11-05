#include "../../../../../Utility/AsoUtility.h"
#include "../../../../../Manager/SoundManager.h"
#include "../../../../../Manager/ResourceManager.h"
#include "../../../../../Manager/InputManager.h"
#include "../../../../../Manager/SceneManager.h"
#include "../../../UI/UISt.h"
#include "../Player.h"
#include "PlayerMoveState.h"

PlayerMoveState::PlayerMoveState(void)
	: moveType_(MOVE_TYPE::NONE)
	, nextMoveType_(MOVE_TYPE::NONE)
{
}

PlayerMoveState::~PlayerMoveState(void)
{
}

void PlayerMoveState::StartState(Player& player)
{
	// 移動タイプを歩きに設定
	SetWalk(player);
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

	// ダッシュフラグを取得して、移動タイプを設定
	bool canDash = player.GetInputDash() && player.stamina_ > 0.0f;
	nextMoveType_ = canDash ? MOVE_TYPE::RUN : MOVE_TYPE::WALK;

	// 移動タイプを設定
	SetMoveType(player);

	// playerの移動方向を更新
	player.moveDir_ = inputDir;

	// 移動量の更新
	player.movePow_ =
		VAdd(player.movePow_, VScale(player.moveDir_, player.moveSpeed_));

	// 移動量が移動速度を超えないように制限
	if (VSize(player.movePow_) > player.moveSpeed_)
	{
		// 移動量を正規化して移動速度にスケーリング
		player.movePow_ = VScale(VNorm(player.movePow_), player.moveSpeed_);
	}
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

void PlayerMoveState::SetMoveType(Player& player)
{
	// すでに同じ移動タイプの場合は処理しない
	if (moveType_ == nextMoveType_) return;

	// 移動タイプを更新
	moveType_ = nextMoveType_;
	nextMoveType_ = MOVE_TYPE::NONE;

	// 移動タイプに応じた処理を実行
	if (moveType_ == MOVE_TYPE::RUN)
	{
		SetRun(player);
	}
	else
	{
		SetWalk(player);
	}
}