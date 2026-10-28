#pragma once
#include <map>
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

    // サウンド登録
    void Add(SOUND_ID soundId, int bgmHandle, int volume = MaxVolumeValue);

    // 解放処理
    void Release(void);

    // ユーザーが指定する最大音量
    static constexpr int MaxVolumeValue = 100;  
    // DxLib内部で扱われる最大音量
    static constexpr int DxLibMaxVolume = 255; 

    // BGMのループ再生
    void PlaySound(SOUND_ID soundId, bool isLoop = false);
    // BGMの一時停止
    void PauseBGM();
    // BGMの再開
    void ResumeSound();
    // BGMの完全停止とメモリ解放
    void StopSound();
    // 再生中のBGM音量変更
    void SetVolume(int volume);
    // SE音の流る速度を設定
    void SetSpeed(SOUND_ID id, float speed);
  
private:
	// シングルトンインスタンス
    SoundManager(void);
    // コピーコンストラクタも同様
    SoundManager(const SoundManager& instance) = default;
    ~SoundManager(void) = default;

    // 一時停止フラグ
    bool isBgmPaused_;

    // SEをIDで管理するためのマップ
    std::map<SOUND_ID, Sound> sounds_;

    // 0-100の値をDxLib用の0-255に変換する内部関数
    int CalcVolume(int volume) const { return (DxLibMaxVolume * volume) / MaxVolumeValue; }
};