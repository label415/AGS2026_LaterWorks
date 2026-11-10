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
	player.stamina_ -= player.param_.consumptionStEvasion;

	// 回避開始時の回転を保存
	lastQrot_ = player.transform_.quaRotLocal;
	player.transform_.quaRotLocal =
		Quaternion::Mult(player.transform_.quaRotLocal, Quaternion::AngleAxis(
			AsoUtility::Deg2RadF(player.param_.avoidanceAngleAxisY), AsoUtility::AXIS_Y));

	// 無敵状態にする
	player.invincibleTimer_ = player.param_.invincibleTime;

	// 回避速度を設定
	player.moveSpeed_ = player.param_.speedEvasion;

	// 回避アニメーション再生
	player.animationController_->Play(static_cast<int>(Player::ANIM_TYPE::EVASION), false);

	// 回避SE再生
	player.sngMng_.PlaySoundId(SoundManager::SOUND_ID::PLAYER_AVE);
}

void PlayerEvasionState::Update(Player& player)
{
	// 移動処理
	player.movePow_ = VScale(player.moveDir_, player.moveSpeed_);

	// アニメーションが終了したら待機状態に遷移
	if (player.animationController_->IsEnd())
	{
		// 回避終了時の回転を復元
		player.transform_.quaRotLocal = lastQrot_;

		// 待機状態に遷移
		player.ChangeState(Player::STATE::IDLE);
	}
}

void PlayerEvasionState::EndState(Player& player)
{
}
