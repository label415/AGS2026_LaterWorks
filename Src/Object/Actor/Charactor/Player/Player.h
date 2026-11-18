#pragma once
#include <functional>
#include "../../../../Utility/JsonUtility.h"
#include "../StateBase.h"
#include "../CharactorBase.h"

class Camara;
class UISt;
class UIRecovery;

class Player :
	public CharactorBase
{

	friend class PlayerIdleState;
	friend class PlayerMoveState;
	friend class PlayerAttackState;
	friend class PlayerEvasionState;
	friend class PlayerDamageState;
	friend class PlayerRecoveryState;

public:

	// アニメーションタイプ
	enum class ANIM_TYPE
	{
		IDLE,
		RUN,
		FAST_RUN,
		ROLLING,
		ATTACK_1,
		ATTACK_2,
		ATTACK_3,
		EVASION,
		DOWN,
		UP,
		RECOVERY,
	};

	// プレイヤー状態
	enum class STATE
	{
		IDLE,
		MOVE,
		ATTACK,
		EVASION,
		DAMAGE,
		RECOVERY,
		DIE,
		END,
	};

	// 各部位のタグ
	enum class EFFECT {
		NONE,
		BLOOD,
		HP_ABSOLUTE,
	};

	// 攻撃コンボ状態
	enum class STATE_ATTACK_COMBO
	{
		COMBO_1,
		COMBO_2,
		COMBO_3,
		MAX
	};

	// 攻撃コンボ制御
	struct ATTACK_COMBO
	{
		ANIM_TYPE animType = ANIM_TYPE::IDLE;
		float stepInputStart = 0.0f;
		float stepInputEnd = 0.0f;
		float stepCollisionStart = 0.0f;
		float stepCollisionEnd = 0.0f;
		float stepInterrupt = 0.0f;
		float moveSpeed = 0.0f;
		STATE_ATTACK_COMBO nextCombo = STATE_ATTACK_COMBO::MAX;
		bool isNextCombo = false;
		float step = 0.0f;

		bool IsValidCombo(float s) const { return s > stepInputStart && s < stepInputEnd; }
		bool IsValidCollsion(float s) const { return s > stepCollisionStart && s < stepCollisionEnd; }
		bool IsValidInterrupt(float s) const { return s > stepInterrupt; }
	};

	// コンストラクタ
	Player(void);

	// デストラクタ
	~Player(void);

	// ランタイム用GUI描画(現在座標・現在HP確認等)
	void UpdateGui(void) override;

	// 描画処理
	void Draw(void) override;

	// リソースの解放処理
	void Release(void) override;

	// 無敵状態かどうかを取得
	const bool GetIsInvincible(void) const { return invincibleTimer_ > 0.0f; }

	// スタミナを消費できるか判定する
	const bool GetCanConsumeStamina(void) const { return stamina_ > 0.0f && stCoolTime_ <= 0.0f; }

	// スタミナを取得
	const float GetStamina(void) const { return stamina_; }

	// HPを取得
	const float& GetHp(void) const { return hp_; }

	// PlayerData.jsonへの直接参照を取得
	Json& GetData(void) const;

protected:

	// プレイヤーリソースロード
	void InitLoad(void) override;

	// 大きさ、回転、座標のトランスフォーム初期化
	void InitTransform(void) override;

	// コライダー（線分・カプセル）の初期化
	void InitCollider(void) override;

	// アニメーションコントローラーの構築とアニメーション登録
	void InitAnimation(void) override;

	// ロード後のコンボパラメータや内部変数の初期化
	void InitPost(void) override;

	// プレイヤーの主更新処理
	void UpdateProcess(void) override;

	// プレイヤー更新後の後処理
	void UpdateProcessPost(void) override;

	// 当たり判定衝突時の更新処理
	void UpdateHitCollider(void) override;

private:

	// 各状態クラスのインスタンスを保持するマップ
	std::map<int, std::unique_ptr<StateBase<Player>>> states_;

	// 状態クラス
	StateBase<Player>* currentState_;

	// 状態
	int state_;

	// 状態遷移
	void ChangeState(int state);

	// 攻撃コンボデータ
	std::map<STATE_ATTACK_COMBO, ATTACK_COMBO> atkComboData_;

	// 現在の攻撃コンボ状態
	STATE_ATTACK_COMBO stateAtkCombo_;

	// 攻撃コンボの更新ステップ
	bool isComboNext_;

	// エフェクト種別
	EFFECT effType_;

	// 無敵時間
	float invincibleTimer_;

	// スタミナ
	float stamina_;

	// スタミナ硬直時間
	float stCoolTime_;

	// 回復瓶数
	int recoveryBottleCount_;

	// アニメーションを固定化する座標
	VECTOR animLockPos_;

	// JSONの変更をコライダー・アニメ速度・コンボ等に即時反映する
	void ApplyJson(void);

	// コンボデータをJSONから読み込む
	void LoadComboFromJson(void);

	// 武器初期化処理
	void InitWeapon(void);

	// サウンド初期化処理
	void InitSound(void);

	// ステート初期化処理
	void InitState(void);

	// 無敵処理
	void ProcessVinclible(void);

	// スタミナ回復処理
	void ProcessStaminaRecovery(void);

	// 移動入力方向を取得
	VECTOR GetInputMoveDir(void);

	// ダッシュ入力を取得
	bool GetInputDash(void);

	// 攻撃入力を取得
	bool GetInputAttack(void);

	// 回避入力を取得
	bool GetInputEvasion(void);

	// 回復入力を取得
	bool GetInputRecovery(void);

	// 入力に応じた次の状態を取得
	STATE GetNextStateByInput(void);
};
