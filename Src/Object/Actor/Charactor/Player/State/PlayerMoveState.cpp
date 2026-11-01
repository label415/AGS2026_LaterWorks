#include "../../../../../Utility/AsoUtility.h"
#include "../../../../../Manager/SoundManager.h"
#include "../../../../../Manager/ResourceManager.h"
#include "../../../../../Manager/InputManager.h"
#include "../../../../../Manager/SceneManager.h"
#include "../../../UI/UISt.h"
#include "../Player.h"
#include "PlayerMoveState.h"

PlayerMoveState::PlayerMoveState(void)
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

	// アニメーション再生
	player.animationController_->Play(static_cast<int>(Player::ANIM_TYPE::RUN));

	// 歩きSE再生
	player.sngMng_.PlaySoundId(SoundManager::SOUND_ID::PLAYER_WAKE, true);
}

void PlayerMoveState::Update(Player& player)
{
	// 共通化した関数から方向を取得
	VECTOR inputDir = player.GetInputDirection();

	// 入力がない場合は、移動状態を終了して待機状態に遷移
	if (AsoUtility::EqualsVZero(inputDir))
	{
		player.ChangeState(Player::STATE::IDLE);
		return;
	}

	player.moveDir_ = inputDir;

	// ダッシュ入力判定
	auto& ins = InputManager::GetInstance();
	bool isR = ins.IsNew(KEY_INPUT_LSHIFT)
		|| ins.IsPadBtnNew(InputManager::JOYPAD_NO::PAD1, InputManager::JOYPAD_BTN::DOWN);

	// スタミナが回復中で、かつスタミナが残っている場合はダッシュ可能
	if (isR && player.stRecoverTime_ <= 0.0f) {
		player.moveSpeed_ = player.SPEED_DASH;
		player.uiSt_->SetSt(player.CONSUMPTION_ST_FAST_RUN * player.scnMng_.GetDeltaTime());
		player.animationController_->Play(static_cast<int>(Player::ANIM_TYPE::FAST_RUN));
	}
	else {
		
		SoundManager::GetInstance().StopSE(SoundManager::SE_ID::PLAYER_RAN);
		int bgm_ = resMng_.Load(ResourceManager::SRC::SE_PLAYER_WAKE).handleId_;
		SoundManager::GetInstance().PlayLoopSE(SoundManager::SE_ID::PLAYER_WAKE, bgm_, SE_VOLUME_WALK);
	}

	movePow_ = VAdd(movePow_, VScale(moveDir_, moveSpeed_));
	float currentSpeed = VSize(movePow_);
	if (currentSpeed > 100.0f)
	{
		movePow_ = VScale(VNorm(movePow_), 100.0f);
	}

	//移動量の減速処理
	movePow_ = AsoUtility::Lerp(movePow_, AsoUtility::VECTOR_ZERO, 0.0001f);
}

void PlayerMoveState::EndState(Player& player)
{
	// 歩きSE停止
	player.sngMng_.StopSound(SoundManager::SOUND_ID::PLAYER_WAKE);
}

void PlayerMoveState::SetWalk(void)
{
	player.moveSpeed_ = player.SPEED_MOVE;
	player.animationController_->Play(static_cast<int>(Player::ANIM_TYPE::RUN));
}

void PlayerMoveState::SetRun(void)
{
	player.moveSpeed_ = player.SPEED_DASH;
}
