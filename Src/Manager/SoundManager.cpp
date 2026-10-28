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
    sounds_.clear();
}

void SoundManager::PlaySound(SOUND_ID soundId, bool isLoop)
{
    if (sounds_[soundId].handle == -1 
        && )return;

    SetVolume(sounds_[soundId].volume);
    PlaySoundMem(sounds_[soundId].handle, DX_PLAYTYPE_LOOP, TRUE);
}

void SoundManager::PauseBGM()
{

}

void SoundManager::ResumeSound()
{
}

void SoundManager::StopSound()
{
}

void SoundManager::SetVolume(int volume)
{
}

void SoundManager::SetSpeed(SOUND_ID id, float speed)
{
}

void SoundManager::ResumeBGM()
{
    // 一時停止フラグが立っている場合のみ再開
    if (bgmHandle_ != -1 && isBgmPaused_)
    {
        
        PlaySoundMem(bgmHandle_, DX_PLAYTYPE_LOOP, FALSE);
        isBgmPaused_ = false;
    }
}

void SoundManager::StopBGM()
{
	// BGMが再生中の場合のみ停止処理を行う
    if (bgmHandle_ != -1)
    {
        StopSoundMem(bgmHandle_);     
        DeleteSoundMem(bgmHandle_);   
        bgmHandle_ = -1;
        isBgmPaused_ = false;
    }
}

void SoundManager::SetBGMVolume(int volume)
{
	//  BGMが再生中の場合のみ音量変更を行う
    if (bgmHandle_ != -1) {
        
        ChangeVolumeSoundMem(CalcVolume(volume), bgmHandle_);
    }
}

void SoundManager::PlaySlowBGM(int handle,int slow)
{
	// 既存のBGMを停止して解放
    StopBGM();
    bgmHandle_ = DuplicateSoundMem(handle);
    if (bgmHandle_ != -1)
    {
       
        int freq = GetFrequencySoundMem(bgmHandle_);
        SetFrequencySoundMem(freq / slow, bgmHandle_);
        PlaySoundMem(bgmHandle_, DX_PLAYTYPE_LOOP, TRUE);
    }
}

void SoundManager::PlaySE(SE_ID id, int handle, int volume)
{
    // 同じIDのSEが既に鳴っていれば停止（連打時のノイズ防止や音の重なり制御）
    StopSE(id);

    int newSe = DuplicateSoundMem(handle);
    if (newSe != -1)
    {
        ChangeVolumeSoundMem(CalcVolume(volume), newSe);
        PlaySoundMem(newSe, DX_PLAYTYPE_BACK, TRUE); // 非同期で再生
        seHandles_[id] = newSe; // IDをキーにしてハンドルを保存
    }
}

void SoundManager::PlayLoopSE(SE_ID id, int handle, int volume)
{
    if (CheckSoundMem(seHandles_[id]) == 1)return;

    StopSE(id);

    int newSe = DuplicateSoundMem(handle);
    if (newSe != -1)
    {
        ChangeVolumeSoundMem(CalcVolume(volume), newSe);
        PlaySoundMem(newSe, DX_PLAYTYPE_LOOP, TRUE); // 非同期で再生
        seHandles_[id] = newSe; // IDをキーにしてハンドルを保存
    }
}

void SoundManager::StopSE(SE_ID id)
{
    // マップ内に指定されたIDが存在するか確認
    if (seHandles_.count(id))
    {
        StopSoundMem(seHandles_[id]);   // 再生停止
        DeleteSoundMem(seHandles_[id]); // メモリ解放
        seHandles_.erase(id);           // リストから削除
    }
}

void SoundManager::AllStopSE()
{
    // 管理されているすべてのSEをループで停止・解放
    for (auto& pair : seHandles_) {
        StopSoundMem(pair.second);
        DeleteSoundMem(pair.second);
    }
    seHandles_.clear();
}

void SoundManager::SetSESpeed(SE_ID id, float speed)
{
	// SEが再生中であることを確認
    int freq = GetFrequencySoundMem(seHandles_[id]);
    SetFrequencySoundMem(static_cast<int>(freq * speed), seHandles_[id]);
}

void SoundManager::PlaySlowSE(SE_ID id, int handle, int volume,int slow)
{
	// 既存のSEを停止して解放
    StopSE(id);
    int newSe = DuplicateSoundMem(handle);
    if (newSe != -1)
    {
        ChangeVolumeSoundMem(CalcVolume(volume), newSe);
        // 周波数を下げて「スロー演出（鈍い音）」にする
        int freq = GetFrequencySoundMem(newSe);
        SetFrequencySoundMem(freq / slow, newSe);
        PlaySoundMem(newSe, DX_PLAYTYPE_BACK, TRUE);
        seHandles_[id] = newSe;
    }
}

SoundManager::SoundManager(void)
    : bgmHandle_(-1)
    ,currentBgmVol_(DxLibMaxVolume)
    , isBgmPaused_(false)
{
    // 配列初期化
    seHandles_.clear();
}