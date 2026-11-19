#include <DxLib.h>
#include <math.h>
#include "../../../../../Utility/AsoUtility.h"
#include "../EnemyBoss.h"
#include "BossChaseState.h"

namespace
{
	constexpr const wchar_t* ROOT_BONE_NAME = L"mixamorig:Hips";

	float WrapRadPi(float rad)
	{
		while (rad > DX_PI_F)  rad -= DX_TWO_PI_F;
		while (rad < -DX_PI_F) rad += DX_TWO_PI_F;
		return rad;
	}
}

BossChaseState::BossChaseState(void)
	: hipsFrameNo_(-1)
	, currentAnimType_(-1)
	, hasAnimYaw_(false)
	, baseAnimYaw_(0.0f)
	, prevAnimYaw_(0.0f)
	, prevAnimStep_(0.0f)
{
}

void BossChaseState::StartState(EnemyBoss& boss)
{
	// ルートボーン(mixamorig:Hips)のフレーム番号を取得
	hipsFrameNo_ = MV1SearchFrame(boss.transform_.modelId, ROOT_BONE_NAME);
	currentAnimType_ = -1;
	hasAnimYaw_ = false;
	baseAnimYaw_ = 0.0f;
	prevAnimYaw_ = 0.0f;
	prevAnimStep_ = 0.0f;

	// 移動慣性を止め、現在の水平正面方向を維持する
	boss.movePow_ = AsoUtility::VECTOR_ZERO;
	VECTOR forward = boss.transform_.GetForward();
	forward.y = 0.0f;
	boss.moveDir_ = AsoUtility::VNormalize(forward);

	// 対象を正面にとらえていなければ、左右判定して回転アニメーションを開始する
	if (boss.blackboard_.angleToPlayer > EnemyBoss::FOV_HALF_ANGLE)
	{
		PlayTurnAnimation(boss);
	}
}

void BossChaseState::Update(EnemyBoss& boss)
{
	// 左右どちらに回転するかを確認し、対応するアニメーションを再生する
	// (正面到達時のIDLE遷移はHFSM側で一括管理する)
	PlayTurnAnimation(boss);
}

void BossChaseState::UpdatePost(EnemyBoss& boss)
{
	if (hipsFrameNo_ < 0 || currentAnimType_ < 0)
	{
		return;
	}

	int modelId = boss.transform_.modelId;
	const auto& playAnim = boss.animationController_->GetPlayAnim();
	if (playAnim.attachNo < 0)
	{
		return;
	}

	float deltaYaw = 0.0f;
	float currAnimYaw = 0.0f;

	if (!hasAnimYaw_)
	{
		// 初回はstep=0での基準角度を取得してから現在stepの角度を計測する
		MV1SetAttachAnimTime(modelId, playAnim.attachNo, 0.0f);
		baseAnimYaw_ = GetHipsAnimYaw(boss);

		MV1SetAttachAnimTime(modelId, playAnim.attachNo, playAnim.step);
		currAnimYaw = GetHipsAnimYaw(boss);

		deltaYaw = WrapRadPi(currAnimYaw - baseAnimYaw_);
		hasAnimYaw_ = true;
	}
	else if (playAnim.step < prevAnimStep_)
	{
		// アニメーションがループして先頭に戻ったフレーム
		// 前フレームから終端までの残り回転分と、先頭から現在stepまでの回転分を合算する
		MV1SetAttachAnimTime(modelId, playAnim.attachNo, playAnim.totalTime);
		float endYaw = GetHipsAnimYaw(boss);
		float tailDelta = WrapRadPi(endYaw - prevAnimYaw_);

		MV1SetAttachAnimTime(modelId, playAnim.attachNo, 0.0f);
		baseAnimYaw_ = GetHipsAnimYaw(boss);

		MV1SetAttachAnimTime(modelId, playAnim.attachNo, playAnim.step);
		currAnimYaw = GetHipsAnimYaw(boss);
		float headDelta = WrapRadPi(currAnimYaw - baseAnimYaw_);

		deltaYaw = tailDelta + headDelta;
	}
	else
	{
		// 通常フレーム：前フレームからのボーン回転差分を取得する
		currAnimYaw = GetHipsAnimYaw(boss);
		deltaYaw = WrapRadPi(currAnimYaw - prevAnimYaw_);
	}

	prevAnimYaw_ = currAnimYaw;
	prevAnimStep_ = playAnim.step;

	// ボーンの実際の回転差分をボスの回転情報(quaRot)へ反映する
	Quaternion stepRot = Quaternion::AngleAxis(
		static_cast<double>(deltaYaw), AsoUtility::AXIS_Y);
	boss.transform_.quaRot = Quaternion::Mult(boss.transform_.quaRot, stepRot);
	boss.transform_.quaRot.Normalize();

	// DelayRotateによる戻り回転を防ぎ、水平の正面方向を同期してモデル行列を更新する
	VECTOR forward = boss.transform_.GetForward();
	forward.y = 0.0f;
	boss.moveDir_ = AsoUtility::VNormalize(forward);
	boss.transform_.Update();

	// ボーン側に残っているアニメーションのY軸回転分を打ち消して二重回転を防ぐ
	float cancelYaw = WrapRadPi(currAnimYaw - baseAnimYaw_);
	CancelHipsAnimYaw(boss, cancelYaw);
}

void BossChaseState::EndState(EnemyBoss& boss)
{
	// Hipsのユーザーローカル行列をリセットして他アニメーションへ影響を残さない
	if (hipsFrameNo_ >= 0)
	{
		MV1ResetFrameUserLocalMatrix(boss.transform_.modelId, hipsFrameNo_);
	}
	currentAnimType_ = -1;
	hasAnimYaw_ = false;
}

void BossChaseState::PlayTurnAnimation(EnemyBoss& boss)
{
	auto cross = AsoUtility::GetCross(
		*boss.targetPos_, boss.transform_.pos, boss.transform_.GetForward());

	int nextAnimType = (cross.y >= 0.0f)
		? static_cast<int>(EnemyBoss::ANIM_TYPE::RIGHT_ROT)
		: static_cast<int>(EnemyBoss::ANIM_TYPE::LEFT_ROT);

	if (currentAnimType_ != nextAnimType)
	{
		if (hipsFrameNo_ >= 0)
		{
			MV1ResetFrameUserLocalMatrix(boss.transform_.modelId, hipsFrameNo_);
		}
		currentAnimType_ = nextAnimType;
		hasAnimYaw_ = false;
		boss.animationController_->Play(currentAnimType_, true, true);
	}
}

float BossChaseState::GetHipsAnimYaw(EnemyBoss& boss) const
{
	int modelId = boss.transform_.modelId;
	MV1ResetFrameUserLocalMatrix(modelId, hipsFrameNo_);
	MATRIX matHipsWorld = MV1GetFrameLocalWorldMatrix(modelId, hipsFrameNo_);

	// Hipsのローカル前方(+Z)をワールド空間へ変換（回転+スケールのみ）
	VECTOR dirWorld = VTransformSR(AsoUtility::DIR_F, matHipsWorld);
	dirWorld.y = 0.0f;
	if (VSize(dirWorld) < 1e-4f)
	{
		dirWorld = VTransformSR(AsoUtility::DIR_R, matHipsWorld);
		dirWorld.y = 0.0f;
	}

	// ボスのワールド回転(quaRot)を除いてアニメーション由来のY軸角度を求める
	VECTOR dirLocal = boss.transform_.quaRot.Inverse().PosAxis(dirWorld);
	dirLocal.y = 0.0f;

	return atan2f(dirLocal.x, dirLocal.z);
}

void BossChaseState::CancelHipsAnimYaw(EnemyBoss& boss, float cancelYawRad) const
{
	int modelId = boss.transform_.modelId;
	MV1ResetFrameUserLocalMatrix(modelId, hipsFrameNo_);

	// 親フレームのワールド回転行列を取得（親が無い場合はモデルのワールド行列）
	int parentFrame = MV1GetFrameParent(modelId, hipsFrameNo_);
	MATRIX matParentWorld = (parentFrame >= 0)
		? MV1GetFrameLocalWorldMatrix(modelId, parentFrame)
		: MV1GetMatrix(modelId);

	// 親フレームのワールド行列からスケールを除いた純粋回転行列を作成
	VECTOR parentScl = MGetSize(matParentWorld);
	MATRIX matParentRot = MGetRotElem(matParentWorld);
	if (parentScl.x > 1e-6f && parentScl.y > 1e-6f && parentScl.z > 1e-6f)
	{
		matParentRot = MMult(matParentRot,
			MGetScale(VGet(1.0f / parentScl.x, 1.0f / parentScl.y, 1.0f / parentScl.z)));
	}
	MATRIX matParentRotInv = MTranspose(matParentRot);

	// ワールドY軸まわりの逆回転(-cancelYawRad)をHipsローカル空間の回転へ変換
	MATRIX matCancelWorldY = Quaternion::AngleAxis(
		static_cast<double>(-cancelYawRad), AsoUtility::AXIS_Y).ToMatrix();
	MATRIX matCancelLocal = MMult(MMult(matParentRot, matCancelWorldY), matParentRotInv);

	// Hips本来のローカル行列の回転成分に逆回転を合成（位置・スケールは維持）
	MATRIX matLocal = MV1GetFrameLocalMatrix(modelId, hipsFrameNo_);
	VECTOR localScl = MGetSize(matLocal);
	VECTOR localPos = MGetTranslateElem(matLocal);
	MATRIX matLocalRot = MGetRotElem(matLocal);
	if (localScl.x > 1e-6f && localScl.y > 1e-6f && localScl.z > 1e-6f)
	{
		matLocalRot = MMult(matLocalRot,
			MGetScale(VGet(1.0f / localScl.x, 1.0f / localScl.y, 1.0f / localScl.z)));
	}

	MATRIX matMix = MGetIdent();
	matMix = MMult(matMix, MGetScale(localScl));
	matMix = MMult(matMix, MMult(matLocalRot, matCancelLocal));
	matMix = MMult(matMix, MGetTranslate(localPos));

	MV1SetFrameUserLocalMatrix(modelId, hipsFrameNo_, matMix);
}
