#pragma once
#include <map>
#include <vector>
#include "../Template/Singleton.h"

class SoundManager : public Singleton<SoundManager>
{
	friend class Singleton<SoundManager>;

public:
	// サウンド情報構造体
	struct Sound
	{
		// サウンドハンドル
		int handle;
		// 音量
		int volume;
	};

	// サウンドID
	enum class SOUND_ID
	{
		PLAYER_WEPON_SE1,
		PLAYER_WEPON_SE2,
		PLAYER_WAKE,
		PLAYER_RAN,
		PLAYER_HER,
		PLAYER_AVE,
		PLAYER_DMAGE,

		ENEMY_ROAR,
		ENEMY_WAKE,
		ENEMY_BREASE1,
		ENEMY_BREASE2,
		ENEMY_ATTCEK,
		ENEMY_ARE,
		ENEMY_ARE_ENEMY_BREASE1,
		ENEMY_ARE_ENEMY_BREASE2,
		ENEMY_FALL,

		BGM_TITLI,
		BGM_GAME,
		Max,
	};

	/// <summary>
	///  サウンド登録
	/// </summary>
	/// <param name="soundId">サウンドID</param>
	/// <param name="bgmHandle">サウンドハンドル</param>
	/// <param name="volume">音量</param>
	void Add(SOUND_ID soundId, int bgmHandle, int volume = MaxVolumeValue);

	/// <summary>
	/// サウンド再生
	/// </summary>
	/// <param name="soundId">サウンドID</param>
	/// <param name="isLoop">ループ再生(デフォルト = false)</param>
	void PlaySoundId(SOUND_ID soundId, bool isLoop = false);

	/// <summary>
	/// 特定のサウンドの停止
	/// </summary>
	/// <param name="soundId">サウンドID</param>
	void StopSound(SOUND_ID soundId);

	/// <summary>
	/// 再生中のBGM音量変更
	/// </summary>
	/// <param name="bgmHandle">サウンドハンドル</param>
	/// <param name="volume">音量</param>
	void SetVolume(SOUND_ID soundId, int volume);

	/// <summary>
	/// SE音の流る速度を設定
	/// </summary>
	/// <param name="soundId">サウンドハンドル</param>
	/// <param name="speed">再生速度</param>
	void SetSpeed(SOUND_ID soundId, float speed);

	// すべてのサウンドの停止
	void AllStopSound(void);

	// 解放処理
	void Release(void);

private:

	// ユーザーが指定する最大音量
	static constexpr int MaxVolumeValue = 100;
	// DxLib内部で扱われる最大音量
	static constexpr int DxLibMaxVolume = 255;

	// シングルトンインスタンス
	SoundManager(void);
	// コピーコンストラクタも同様
	SoundManager(const SoundManager& instance) = default;
	~SoundManager(void) = default;

	// SEをIDで管理するためのマップ
	std::map<SOUND_ID, Sound> sounds_;

	// 0-100の値をDxLib用の0-255に変換する内部関数
	int CalcVolume(int volume) const { return (DxLibMaxVolume * volume) / MaxVolumeValue; }
};