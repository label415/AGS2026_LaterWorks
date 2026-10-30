#include <DxLib.h>
#include "SoundManager.h"

void SoundManager::Add(SOUND_ID soundId, int bgmHandle, int volume)
{
	// BGM情報を格納
	sounds_[soundId].handle = bgmHandle;
	sounds_[soundId].volume = volume;
}

void SoundManager::Release(void)
{
	// 動的配列をクリア
	sounds_.clear();
}

void SoundManager::PlaySoundId(SOUND_ID soundId, bool isLoop)
{
	// サウンドがないまたは再生中であれば処理しない
	if (sounds_[soundId].handle == -1 
		|| CheckSoundMem(sounds_[soundId].handle))return;

	// サウンドを再生させる
	SetVolume(soundId, sounds_[soundId].handle);
	PlaySoundMem(sounds_[soundId].handle, isLoop, TRUE);
}

void SoundManager::StopSound(SOUND_ID soundId)
{
	// サウンドがないまたは再生中であれば処理しない
	if (sounds_[soundId].handle == -1
		|| !CheckSoundMem(sounds_[soundId].handle))return;

	// サウンドを停止
	StopSoundMem(sounds_[soundId].handle);
}

void SoundManager::AllStopSound(void)
{
	// すべてのサウンドを停止
	for (const auto& sound : sounds_)
	{
		// 停止するハンドル
		int stopHandle = sound.second.handle;
		StopSoundMem(stopHandle);
	}

}

void SoundManager::SetVolume(SOUND_ID id, int volume)
{
	// サウンド音量設定
	ChangeVolumeSoundMem(volume, sounds_[id].handle);
}

void SoundManager::SetSpeed(SOUND_ID id, float speed)
{
	// サウンド再生速度設定
	SetFrequencySoundMem(speed, sounds_[id].handle);
}

SoundManager::SoundManager(void)
{
	// 動的配列をクリア
	sounds_.clear();
}