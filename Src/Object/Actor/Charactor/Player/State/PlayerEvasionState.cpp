#include "../../../../../Utility/AsoUtility.h"
#include "../../../../../Manager/SoundManager.h"
#include "../../../../../Manager/ResourceManager.h"
#include "../../../../../Manager/InputManager.h"
#include "../../../../../Manager/SceneManager.h"
#include "../../../UI/UISt.h"
#include "../Player.h"
#include "PlayerEvasionState.h"

PlayerEvasionState::PlayerEvasionState(void)
{
}

PlayerEvasionState::~PlayerEvasionState(void)
{
}

void PlayerEvasionState::StartState(Player& player)
{
	// 回避開始時の回転を保存
	player.lastQrot_ = player.transform_.quaRotLocal;
	player.transform_.quaRotLocal =
		Quaternion::Mult(player.transform_.quaRotLocal, Quaternion::AngleAxis(
			AsoUtility::Deg2RadF(Player::AVOIDANCE_ANGLE_AXIS_Y), AsoUtility::AXIS_Y));

	// 無敵状態にする
	player.invincibleTimer_ = Player::INVINCIBLE_TIME;

	// 回避速度を設定
	player.moveSpeed_ = Player::SPEED_EVASION;

	// 回避アニメーション再生
	player.animationController_->Play(static_cast<int>(Player::ANIM_TYPE::EVASION), false);

	// 回避SE再生
	player.sngMng_.PlaySoundId(SoundManager::SOUND_ID::PLAYER_AVE, true);
}

void PlayerEvasionState::Update(Player& player)
{
	// 移動処理
	player.movePow_ = VScale(player.moveDir_, player.moveSpeed_);

	// アニメーションが終了したら待機状態に遷移
	if (player.animationController_->IsEnd())
	{
		// 回避終了時の回転を復元
		player.transform_.quaRotLocal = player.lastQrot_;

		// 待機状態に遷移
		player.ChangeState(Player::STATE::IDLE);
	}
}

void PlayerEvasionState::EndState(Player& player)
{
}
