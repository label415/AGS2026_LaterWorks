#include <DxLib.h>
#include "../../Manager/SceneManager.h"
#include "AnimationController.h"

AnimationController::AnimationController(int modelId)
	:
	modelId_(modelId),
	playType_(-1),
	playAnim_(),
	SpState_(0.0f),
	SpEnd_(0.0f),
	SpecificLoop_(false),
	isLoop_(true),
	isReversing_(false),
	isStop_(false),
	prevAnim_(),
	prevIsLoop_(true),
	isBlending_(false),
	blendRate_(1.0f),
	blendTime_(0.0f)
{
}

AnimationController::~AnimationController(void)
{
}

void AnimationController::Add(int type, float speed, const std::wstring path, float blendTime)
{
	// 外部FBXからアニメーションを追加する
	Animation animation;
	animation.model = MV1LoadModel(path.c_str());
	animation.animIndex = -1;

	Add(type, speed, blendTime, animation);
}

void AnimationController::Add(int type, float speed, int handlId, float blendTime)
{
	// ハンドルIDからアニメーションを追加する
	Animation animation;
	animation.model = handlId;
	animation.animIndex = -1;

	Add(type, speed, blendTime, animation);
}

void AnimationController::AddInFbx(int type, float speed, int animIndex, float blendTime)
{
	// 同じFBX内のアニメーションを準備
	Animation animation;
	animation.model = -1;
	animation.animIndex = animIndex;

	Add(type, speed, blendTime, animation);
}

void AnimationController::Play(int type, float blendTime, bool isLoop, bool isForce)
{
	// 指定されたアニメーションが存在しない場合は処理しない
	if (animations_.find(type) == animations_.end())
	{
		return;
	}

	if (!isForce && playType_ == type)
	{
		// 同じアニメーションだったら再生を継続する
		return;
	}

	// すでにブレンド中なら、一番古いアニメーションを先に外す
	if (prevAnim_.attachNo != -1)
	{
		MV1DetachAnim(modelId_, prevAnim_.attachNo);
		prevAnim_.attachNo = -1;
	}

	// ブレンド時間が未指定(負)なら登録済みのブレンド時間を使う
	float useBlendTime = (blendTime < 0.0f) ? animations_[type].blendTime : blendTime;

	// ブレンド開始できるか判定
	if (useBlendTime > 0.0f && playType_ != -1 && playAnim_.attachNo != -1)
	{
		// 再生中だったアニメーションを「前のアニメーション」として保持
		prevAnim_ = playAnim_;
		prevIsLoop_ = isLoop_;
		isBlending_ = true;
		blendRate_ = 0.0f;
		blendTime_ = useBlendTime;
	}
	else
	{
		if (playAnim_.attachNo != -1)
		{
			// モデルからアニメーションを外す
			MV1DetachAnim(modelId_, playAnim_.attachNo);
		}
		isBlending_ = false;
		blendRate_ = 1.0f;
		blendTime_ = 0.0f;
	}

	// アニメーション種別を変更
	playType_ = type;
	playAnim_ = animations_[type];

	// 初期化
	playAnim_.step = 0.0f;

	// モデルにアニメーションを付ける
	if (playAnim_.model == -1)
	{
		// モデルと同じファイルからアニメーションをアタッチする
		playAnim_.attachNo = MV1AttachAnim(modelId_, playAnim_.animIndex);
	}
	else
	{
		// 別のモデルファイルからアニメーションをアタッチする
		// DxModelViewerを確認すること(大体0か1)
		int animIdx = 0;
		playAnim_.attachNo = MV1AttachAnim(modelId_, animIdx, playAnim_.model);
	}

	// アニメーション総時間の取得
	playAnim_.totalTime = MV1GetAttachAnimTotalTime(modelId_, playAnim_.attachNo);
	MV1SetAttachAnimTime(modelId_, playAnim_.attachNo, playAnim_.step);

	// 初期ブレンド率を設定
	if (isBlending_)
	{
		MV1SetAttachAnimBlendRate(modelId_, prevAnim_.attachNo, 1.0f);
		MV1SetAttachAnimBlendRate(modelId_, playAnim_.attachNo, 0.0f);
	}
	else
	{
		MV1SetAttachAnimBlendRate(modelId_, playAnim_.attachNo, 1.0f);
	}

	// アニメーションループ
	isLoop_ = isLoop;
}

void AnimationController::Update(void)
{
	if (isStop_ || playAnim_.attachNo == -1)return;
	// 経過時間の取得
	float deltaTime = SceneManager::GetInstance().GetDeltaTime();

	if (SpecificLoop_)
	{
		// 1. 反転の判定（境界チェック）
		if (playAnim_.step >= SpEnd_)
		{
			playAnim_.step = SpEnd_;
			isReversing_ = true;  // 端に達したので「逆再生」にする
		}
		else if (playAnim_.step <= SpState_)
		{
			playAnim_.step = SpState_;
			isReversing_ = false; // 開始点に戻ったので「通常再生」にする
		}

		// 2. 現在の方向に基づいてステップを進める/戻す
		if (isReversing_)
		{
			playAnim_.step -= (deltaTime * playAnim_.speed);
		}
		else
		{
			playAnim_.step += (deltaTime * playAnim_.speed);
		}

		// アニメーション設定
		MV1SetAttachAnimTime(modelId_, playAnim_.attachNo, playAnim_.step);
	}
	else
	{
		StepAnimation(playAnim_, isLoop_, deltaTime);
	}

	// ブレンド処理
	if (isBlending_)
	{
		// 前のアニメーションも進めながらフェードアウト
		StepAnimation(prevAnim_, prevIsLoop_, deltaTime);

		// ブレンド率を進める
		blendRate_ += deltaTime / blendTime_;

		if (blendRate_ >= 1.0f)
		{
			// ブレンド完了：前のアニメーションをデタッチ
			blendRate_ = 1.0f;
			isBlending_ = false;

			if (prevAnim_.attachNo != -1)
			{
				MV1DetachAnim(modelId_, prevAnim_.attachNo);
				prevAnim_.attachNo = -1;
			}
			MV1SetAttachAnimBlendRate(modelId_, playAnim_.attachNo, 1.0f);
		}
		else
		{
			// 前アニメと新アニメを合成
			MV1SetAttachAnimBlendRate(modelId_, prevAnim_.attachNo, 1.0f - blendRate_);
			MV1SetAttachAnimBlendRate(modelId_, playAnim_.attachNo, blendRate_);
		}
	}
}

void AnimationController::Release(void)
{
	// 外部FBXのモデル(アニメーション)解放
	for (const std::pair<int, Animation>& pair : animations_)
	{
		if (pair.second.model != -1)
		{
			MV1DeleteModel(pair.second.model);
		}
	}

	// 可変長配列をクリアする
	animations_.clear();

	playType_ = -1;
	playAnim_ = Animation();
	prevAnim_ = Animation();
	isBlending_ = false;
	blendRate_ = 1.0f;
	blendTime_ = 0.0f;
}

int AnimationController::GetPlayType(void) const
{
	return playType_;
}

int AnimationController::GetPlayType(int type) const
{
	auto it = animations_.find(type);
	if (it == animations_.end()) {
		return -1;
	}

	return playType_;
}

bool AnimationController::IsEnd(void) const
{
	bool ret = false;

	if (isLoop_)
	{
		// ループ設定されているなら、
		// 無条件で終了しないを返す
		return ret;
	}

	if (playAnim_.step >= playAnim_.totalTime)
	{
		// 再生時間を過ぎたらtrue
		return true;
	}

	return ret;
}

bool AnimationController::IsEnd(int type) const
{
	auto it = animations_.find(type);
	if (it == animations_.end()) {
		return false;
	}

	bool ret = false;

	if (isLoop_)
	{
		// ループ設定されているなら、
		// 無条件で終了しないを返す
		return ret;
	}

	if (playAnim_.step >= playAnim_.totalTime)
	{
		// 再生時間を過ぎたらtrue
		return true;
	}

	return ret;

}

bool AnimationController::IsBlending(void) const
{
	return isBlending_;
}

float AnimationController::GetBlendRate(void) const
{
	return blendRate_;
}

const AnimationController::Animation& AnimationController::GetPlayAnim(void) const
{
	return playAnim_;
}

const AnimationController::Animation& AnimationController::GetPlayAnim(int type) const
{
	// 指定されたアニメーションが存在しない場合は処理しない
	auto it = animations_.find(type);
	if (it == animations_.end()) {
		return Animation();
	}
	return playAnim_;
}

void AnimationController::SetSpecificTime(float state, float end, bool SpecificLoop)
{
	// 特定の時間をループするアニメーションを再生
	SpState_ = state;
	SpEnd_ = end;
	SpecificLoop_ = SpecificLoop;
}

void AnimationController::SetStateTime(float state)
{
	// アニメーションを始める時間を設定
	playAnim_.step = state;
}

void AnimationController::SetIsStopFlager(bool isStop)
{
	// アニメーションを止めるフラグを設定
	isStop_ = isStop;
}

void AnimationController::SetSpeed(int type, float speed)
{
	auto it = animations_.find(type);
	if (it != animations_.end())
	{
		it->second.speed = speed;
	}
	if (playType_ == type)
	{
		playAnim_.speed = speed;
	}
}

void AnimationController::SetBlendTime(int type, float blendTime)
{
	float clamped = (blendTime > 0.0f) ? blendTime : 0.0f;
	auto it = animations_.find(type);
	if (it != animations_.end())
	{
		it->second.blendTime = clamped;
	}
	if (playType_ == type)
	{
		playAnim_.blendTime = clamped;
	}
}

float AnimationController::GetBlendTime(int type) const
{
	auto it = animations_.find(type);
	if (it != animations_.end())
	{
		return it->second.blendTime;
	}
	return 0.0f;
}

void AnimationController::Add(int type, float speed, float blendTime, Animation& animation)
{
	// アニメーション追加の共通処理
	animation.speed = speed;
	animation.blendTime = (blendTime > 0.0f) ? blendTime : 0.0f;

	if (animations_.count(type) == 0)
	{
		// 追加
		animations_.emplace(type, animation);
	}
}

void AnimationController::StepAnimation(Animation& anim, bool isLoop, float deltaTime)
{
	if (anim.attachNo == -1)
	{
		return;
	}

	// 再生
	anim.step += (deltaTime * anim.speed);

	// アニメーションが終了したら
	if (anim.step > anim.totalTime)
	{
		if (isLoop)
		{
			// ループ再生
			anim.step = 0.0f;
		}
		else
		{
			// ループしない
			anim.step = anim.totalTime;
		}
	}

	// アニメーション設定
	MV1SetAttachAnimTime(modelId_, anim.attachNo, anim.step);
}