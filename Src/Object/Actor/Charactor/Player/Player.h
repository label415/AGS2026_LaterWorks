#pragma once
#include <functional>
#include "../../../../Manager/Resource.h"
#include "../CharactorBase.h"

class Camara;
class WeponBase;
class UISt;
class UIRecovery;
class PlayerStateBase;

class Player :
	public CharactorBase
{
	// 各状態クラスからPlayerの内部にアクセスできるようにする
	friend class PlayerStateBase;
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
		// アニメーション種別
		ANIM_TYPE animType = ANIM_TYPE::IDLE;
		// コンボ受付開始ステップ
		float stepInputStart = 0.0f;
		// コンボ受付終了ステップ
		float stepInputEnd = 0.0f;
		// 衝突判定開始ステップ
		float stepCollisionStart = 0.0f;
		// 衝突判定終了ステップ
		float stepCollisionEnd = 0.0f;
		// アニメーション割り込みステップ
		float stepInterrupt = 0.0f;
		// 移動入力時の移動速度
		float moveSpeed = 0.0f;
		// 次のコンボ状態
		STATE_ATTACK_COMBO nextCombo = STATE_ATTACK_COMBO::MAX;
		// 次のコンボに繋げるか
		bool isNextCombo = false;
		// 更新ステップ
		float step = 0.0f;

		// コンボ受付有効ステップ
		bool IsValidCombo(float step) const
		{
			return step > stepInputStart
				&& step < stepInputEnd;
		}

		// 衝突判定有効ステップ
		bool IsValidCollsion(float step) const
		{
			return step > stepCollisionStart
				&& step < stepCollisionEnd;
		}

		// 割り込み有効ステップ
		bool IsValidInterrupt(float step) const
		{
			return step > stepInterrupt;
		}
	};

	// データ駆動パラメータ構造体(PlayerData.jsonと対応)
	struct PARAM
	{
		// Transform初期値
		VECTOR initPos = { 0.0f, 0.0f, -1000.0f };
		VECTOR initScale = { 1.0f, 1.0f, 1.0f };
		float initAngleY = 180.0f;

		// コライダー設定
		VECTOR colLineStart = { 0.0f, 80.0f, 0.0f };
		VECTOR colLineEnd = { 0.0f, -10.0f, 0.0f };
		VECTOR colCapsuleTop = { 0.0f, 110.0f, 0.0f };
		VECTOR colCapsuleDown = { 0.0f, 30.0f, 0.0f };
		float colCapsuleRadius = 20.0f;

		// ステータス関連
		float maxHp = 100.0f;
		float maxStamina = 100.0f;
		float recoveryStSpeed = 20.0f;
		float consumptionStFastRun = 20.0f;
		float consumptionStEvasion = 40.0f;
		float consumptionStAttack = 23.0f;
		float staminaExhaustionTime = 1.5f;
		float invincibleTime = 3.0f;
		float healHpAmount = 40.0f;
		int initialRecoveryBottleCount = 6;
		int consumeRecoveryBottleCount = 1;

		// 移動・回避・補間関連
		float speedMove = 8.0f;
		float speedDash = 12.0f;
		float speedEvasion = 10.0f;
		float movePowDecrease = 0.25f;
		float avoidanceAngleAxisY = 100.0f;
		float rotTargetSlerpRate = 0.1f;
		float attackStepLerpRate = 0.2f;

		// 戦闘・被ダメージ関連
		float damageEnemyBody = 20.0f;
		float damageEnemyWeapon = 35.0f;
		int pushBackPower = 20;

		// エフェクト・サウンド・フレーム関連
		int seVolume = 50;
		VECTOR effectBloodScale = { 5.0f, 5.0f, 5.0f };
		float effectRecoveryScale = 10.0f;
		float effectRecoveryOffsetY = 100.0f;
		float dieFadeSpeed = 0.3f;
		float dieEndThreshold = 1.8f;
		float attackSeTriggerStep = 10.0f;
		float recoveryTriggerStep = 10.0f;
		int weaponAttachFrameNo = 46;
		int hitEffectFrameNo = 2;
		float upLockPosYAdd = 0.7f;
		VECTOR lockPos1 = { 0.0f, 78.0f, 0.0f };
		int lockFrameNo = 1;

		// アニメーション再生速度
		float animSpeedIdle = 30.0f;
		float animSpeedRun = 25.0f;
		float animSpeedFastRun = 30.0f;
		float animSpeedAttack1 = 40.0f;
		float animSpeedAttack2 = 40.0f;
		float animSpeedAttack3 = 40.0f;
		float animSpeedEvasion = 80.0f;
		float animSpeedDown = 50.0f;
		float animSpeedUp = 100.0f;
		float animSpeedRecovery = 40.0f;
	};

	// コンストラクタ
	Player(void);

	// デストラクタ
	~Player(void);

	// GUI更新(統合エディタから呼ばれる)
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

	// パラメータを取得
	const PARAM& GetParam(void) const { return param_; }

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

	// パラメータ
	PARAM param_;

	// 各状態クラスのインスタンスを保持するマップ
	std::map<STATE, std::unique_ptr<PlayerStateBase>> states_;

	// 状態クラス
	PlayerStateBase* currentState_;

	// 状態
	STATE state_;

	// 状態遷移
	void ChangeState(STATE state);

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

	// JSONからパラメータを読み込む
	void LoadParam(const Json& json);

	// パラメータをJSONへ書き出す
	void SaveParam(Json& json) const;

	// パラメータの変更をTransform・コライダー・アニメ速度等へ即時反映する
	void ApplyParam(bool resetTransformPos = false);

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
