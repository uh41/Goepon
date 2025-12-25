/*********************************************************************/
// * \file   sounditembase.cpp
// * \brief  サウンドベースクラス
// *
// * \author 鈴木裕稀
// * \date   2025/12/25
// * \作業内容: 新規作成 鈴木裕稀　2025/12/25
/*********************************************************************/


#include "SoundItemBase.h"
#include "SoundServer.h"

// コンストラクタ
soundserver::SoundItemBase::SoundItemBase(std::string filename, int flag)
{
	_sFileName = filename;
	_iFlag = flag;
	_iSoundHandle = -1;
	_iVolume = 160;
	_iPan = 0;
	_iFrequency = 0;		// ファイルによって標準が違うので
	_soundServer = nullptr;
}

// デストラクタ
soundserver::SoundItemBase::~SoundItemBase()
{
	Unload();
}

void soundserver::SoundItemBase::Unload()
{
	if(_iSoundHandle != -1)
	{
		DeleteSoundMem(_iSoundHandle);
		_iSoundHandle = -1;
	}
}

bool soundserver::SoundItemBase::IsLoad()
{
	if(_iSoundHandle != -1 && CheckHandleASyncLoad(_iSoundHandle) == FALSE)
	{
		return true;
	}
	return false;
}

bool soundserver::SoundItemBase::IsPlay()
{
	if(_iSoundHandle != -1 && CheckSoundMem(_iSoundHandle) == 1)
	{
		return true;
	}
	return false;
}

void soundserver::SoundItemBase::Stop()
{
	if(_iSoundHandle != -1)
	{
		StopSoundMem(_iSoundHandle);
	}
}

void soundserver::SoundItemBase::SetVolume(int volume)
{
	_iVolume = volume;
	if(_iSoundHandle != -1)
	{
		ChangeVolumeSoundMem(_iVolume, _iSoundHandle);
	}
}

void soundserver::SoundItemBase::SetPan(int pan)
{
	_iPan = pan;
	if(_iSoundHandle != -1)
	{
		ChangePanSoundMem(_iPan, _iSoundHandle);
	}
}

void soundserver::SoundItemBase::SetFrequency(int frequency)
{
	_iFrequency = frequency;
	if(_iSoundHandle != -1)
	{
		SetFrequencySoundMem(_iFrequency, _iSoundHandle);
	}
}

void soundserver::SoundItemBase::ResetFrequency()
{
	_iFrequency = 0;
	if(_iSoundHandle != -1)
	{
		ResetFrequencySoundMem(_iSoundHandle);
		_iFrequency = GetFrequencySoundMem(_iSoundHandle);
	}
}

void soundserver::SoundItemBase::PlayMem(int flag)
{
	// 再生前に音量等を設定
	SetVolume(_iVolume);
	SetPan(_iPan);
	SetFrequency(_iFrequency);
	PlaySoundMem(_iSoundHandle, flag, TRUE);
}

int soundserver::SoundItemBase::LoadMem(std::string filename)
{
	if(_iFlag & FLG_3D)
	{
		SetCreate3DSoundFlag(TRUE);
	}
	int sound = LoadSoundMem(filename.c_str());
	SetCreate3DSoundFlag(FALSE);
	return sound;
}
