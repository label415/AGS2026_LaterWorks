#pragma once
#include "../../StateBase.h"

class EnemyBoss;

class BossChaseState : public StateBase<EnemyBoss>
{
public:

	// コンストラクタ
	BossChaseState(void);

	// 開始
	void StartState(EnemyBoss& boss)override;

	// 更新
	void Update(EnemyBoss& boss)override;

	// 更新後処理(アニメーションのボーン回転を回転情報へ同期する)
	void UpdatePost(EnemyBoss& boss)override;

	// 終了
	void EndState(EnemyBoss& boss)override;

private:

	// ルートボーン(mixamorig:Hips)のフレーム番号
	int hipsFrameNo_;

	// 現在再生中の回転アニメーション種別
	int currentAnimType_;

	// アニメーション回転計測済みフラグ
	bool hasAnimYaw_;

	// 現在アニメーションの先頭フレーム(step=0)でのHipsのY軸角度(rad)
	float baseAnimYaw_;

	// 前フレームでのHipsのY軸角度(rad)
	float prevAnimYaw_;

	// 前フレームでのアニメーション再生時間
	float prevAnimStep_;

	// ターゲットの左右に応じて回転アニメーションを切り替える
	void PlayTurnAnimation(EnemyBoss& boss);

	// モデル基準空間でのHipsフレームのY軸角度(rad)を取得する
	float GetHipsAnimYaw(EnemyBoss& boss) const;

	// HipsフレームのワールドY軸まわりの回転のみを逆回転で打ち消す
	void CancelHipsAnimYaw(EnemyBoss& boss, float cancelYawRad) const;

};