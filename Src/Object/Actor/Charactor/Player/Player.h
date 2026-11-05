#pragma once
#include <functional>
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

	//各部位のタグ
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

	// 回避Y軸角度
	static constexpr float AVOIDANCE_ANGLE_AXIS_Y = 100.0f;

	// 移動速度(通常)
	static constexpr float SPEED_MOVE = 8.0f;
	// 移動速度(ダッシュ)
	static constexpr float SPEED_DASH = 12.0f;
	// 移動速度(回避)
	static constexpr float SPEED_EVASION = 10.0f;
	// 移動量減少値
	static constexpr float MOVE_POW_DECREASE = 0.25f;

	// 最大HP量
	static constexpr float MAX_HP = 100.0f;

	// 最大スタミナ量
	static constexpr float MAX_STAMINA = 100.0f;
	// スタミナ回復速度
	static constexpr float RECOVERY_ST_SPEED = 20.0f;
	// ダッシュ時スタミナ消費量
	static constexpr float CONSUMPTION_ST_FAST_RUN = 20.0f;
	// 回避時スタミナ消費量
	static constexpr float CONSUMPTION_ST_EVASION = 40.0f;
	// 攻撃時スタミナ消費量
	static constexpr float CONSUMPTION_ST_ATTACK = 23.0f;

	// スタミナ切れ時の硬直時間
	static constexpr float STAMINA_EXHAUSTION_TIME = 1.5f;

	// 敵本体接触時の被ダメージ
	static constexpr float DAMAGE_ENEMY_BODY = 20.0f;
	// 敵武器接触時の被ダメージ
	static constexpr float DAMAGE_ENEMY_WEAPON = 35.0f;

	// HP回復アイテム使用時の回復量
	static constexpr float HEAL_HP_AMOUNT = 40.0f;
	// 回復ボトル初期所持数
	static constexpr int INITIAL_RECOVERY_BOTTLE_COUNT = 6;
	// 1回あたりの回復ボトル消費量
	static constexpr int CONSUME_RECOVERY_BOTTLE_COUNT = 1;

	// プレイヤー被ダメージSE音量
	static constexpr int SE_VOLUME = 50;

	// 被弾時出血エフェクトスケール
	static constexpr VECTOR EFFECT_BLOOD_SCALE = { 5.0f, 5.0f, 5.0f };
	// 回復エフェクトスケール
	static constexpr float EFFECT_RECOVERY_SCALE = 10.0f;
	// 回復エフェクトY軸オフセット
	static constexpr float EFFECT_RECOVERY_OFFSET_Y = 100.0f;

	// 敵接触時の押し出し量
	static constexpr int PUSH_BACK_POWER = 20;

	// ターゲット追従時の回転Slerp補間係数
	static constexpr float ROT_TARGET_SLERP_RATE = 0.1f;
	// 攻撃踏み込み時のLerp補間係数
	static constexpr float ATTACK_STEP_LERP_RATE = 0.2f;

	// 死亡時のフェード進行速度
	static constexpr float DIE_FADE_SPEED = 0.3f;
	// 死亡ステート完了し終了遷移する閾値
	static constexpr float DIE_END_THRESHOLD = 1.8f;

	// 攻撃時SE再生ステップ
	static constexpr float ATTACK_SE_TRIGGER_STEP = 10.0f;
	// 回復アクション発動ステップ
	static constexpr float RECOVERY_TRIGGER_STEP = 10.0f;

	// 武器アタッチ先フレーム番号
	static constexpr int WEAPON_ATTACH_FRAME_NO = 46;
	// 被弾エフェクト発生フレーム番号
	static constexpr int HIT_EFFECT_FRAME_NO = 2;
	// 起き上がり時LockPosのY軸上昇速度
	static constexpr float UP_LOCK_POS_Y_ADD = 0.7f;

	// コンストラクタ
	Player(void);

	// デストラクタ
	~Player(void);

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
	const float GetHp(void) const { return hp_; }

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
	std::map<STATE, std::unique_ptr<PlayerStateBase>> states_;

	// 現在の状態クラス
	PlayerStateBase* currentState_;

	// 状態
	STATE state_;

	// 遷移予約された状態
	STATE nextState_;

	// 遷移予約があるか
	bool isChangeState_;

	// 状態遷移
	void ChangeState(STATE state);

	// 予約された状態遷移を反映
	void ApplyChangeState(void);

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

	// HP
	float hp_;

	// スタミナ硬直時間
	float stCoolTime_;

	// アニメーションを固定化する座標
	VECTOR animLockPos_;

	//アニメーションを固定化する座標
	static constexpr VECTOR LOCK_POS1 = { 0.0f, 78.0f, 0.0f };

	//アニメーションを固定化するフレーム
	static constexpr int LOCK_FRAME_NO = 1;

	// 衝突判定用線分開始
	static constexpr VECTOR COL_LINE_START_LOCAL_POS = { 0.0f, 80.0f, 0.0f };
	// 衝突判定用線分終了
	static constexpr VECTOR COL_LINE_END_LOCAL_POS = { 0.0f, -10.0f, 0.0f };

	// 衝突判定用カプセル上部球体
	static constexpr VECTOR COL_CAPSULE_TOP_LOCAL_POS = { 0.0f, 110.0f, 0.0f };
	// 衝突判定用カプセル下部球体
	static constexpr VECTOR COL_CAPSULE_DOWN_LOCAL_POS = { 0.0f, 30.0f, 0.0f };
	// 衝突判定用カプセル球体半径
	static constexpr float COL_CAPSULE_RADIUS = 20.0f;

	// 無敵時間
	static constexpr float INVINCIBLE_TIME = 3.0f;

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
};
