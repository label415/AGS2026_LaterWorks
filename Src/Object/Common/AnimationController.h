#pragma once
#include <string>
#include <map>

class AnimationController
{
public:
	// アニメーションデータ
	struct Animation
	{
		int model = -1;
		int attachNo = -1;
		int animIndex = 0;
		float speed = 0.0f;
		float totalTime = 0.0f;
		float step = 0.0f;
		float blendTime = 0.0f;
	};

	// コンストラクタ
	AnimationController(int modelId);

	// デストラクタ
	~AnimationController(void);

	// 外部FBXからアニメーション追加
	void Add(int type, float speed, const std::wstring path, float blendTime = 0.0f);
	void Add(int type, float speed, int handlId, float blendTime = 0.0f);
	
	// 同じFBX内のアニメーションを準備
	void AddInFbx(int type, float speed, int animIndex, float blendTime = 0.0f);

	// アニメーション再生（登録済みのブレンド時間を使用）
	void Play(int type, bool isLoop = true, bool isForce = false);

	// 更新
	void Update(void);

	// 解放
	void Release(void);

	// 再生中のアニメーション
	int GetPlayType(void) const;
	int GetPlayType(int type) const;

	// 再生終了
	bool IsEnd(void) const;
	bool IsEnd(int type) const;

	// ブレンド中かどうか
	bool IsBlending(void) const;

	// 現在のブレンド率を取得
	float GetBlendRate(void) const;

	// 再生中のアニメーション情報を取得
	const Animation& GetPlayAnim(void) const;
	const Animation& GetPlayAnim(int type) const;

	// 特定の時間をループするアニメーションを再生
	void SetSpecificTime(float state, float end, bool SpecificLoop);

	// アニメーションを始める時間を設定
	void SetStateTime(float state);

	// アニメーションを止めるフラグを設定
	void SetIsStopFlager(bool isStop);

	// 指定アニメーションの再生速度を変更
	void SetSpeed(int type, float speed);

	// 指定アニメーションのブレンド時間を変更
	void SetBlendTime(int type, float blendTime);

	// 指定アニメーションのブレンド時間を取得
	float GetBlendTime(int type) const;
private:
	// アニメーションするモデルのハンドルID
	int modelId_;

	// 種類別のアニメーションデータ
	std::map<int, Animation> animations_;

	// 再生中のアニメーション
	int playType_;
	Animation playAnim_;

	//特定の開始時間
	float SpState_;

	//特定の終了時間
	float SpEnd_;

	//特定の時間をループする
	bool SpecificLoop_;

	// アニメーションをループするかしないか
	bool isLoop_;

	// アニメーション追加の共通処理
	void Add(int type, float speed, float blendTime, Animation& animation);

	// アニメーションの再生を開始する共通処理
	bool isReversing_;

	// アニメーションを止めるフラグ
	bool isStop_;

	// ブレンド用：直前に再生していたアニメーション
	Animation prevAnim_;
	bool prevIsLoop_;

	// ブレンド管理
	bool isBlending_;
	float blendRate_;
	float blendTime_;

	// アニメーション時間更新の共通処理
	void StepAnimation(Animation& anim, bool isLoop, float deltaTime);
};