/*********************************************************************/
// * \file   soundserver3D.cpp
// * \brief  3D音源管理クラス
// *
// * \author 鈴木裕稀
/*********************************************************************/

#include "soundserver3D.h"

SoundServer3D::SoundServer3D(at::spc<soundserver::SoundServer> soundServer)
{
	_soundServer = soundServer;
	_radius = sound::DEFAULT_RADIUS;
}

void SoundServer3D::SetRadius(float rad)
{
	_radius = rad;
}

void SoundServer3D::PlayLoopSound3D(void* key, const std::string& soundId, const vec::Vec3& pos)
{
	if(_soundServer == nullptr || key == nullptr || soundId.empty())
	{
		return;
	}

	auto& instance = _instance[key];

	// 初回だけSoundItemBaseを取得してファイル名を保存
	if(instance.sound == nullptr)
	{
		instance.sound = _soundServer->Get(soundId);
		instance.radius = _radius;

		if(instance.sound == nullptr)
		{
			return;
		}

		// ファイル名を取得
		std::string filename = instance.sound->GetFileName();

		// 3Dサウンドフラグを設定
		SetCreate3DSoundFlag(TRUE);

		// 敵ごとに個別のハンドルを作成（メモリ読み込みモード）
		instance.handle = LoadSoundMem(filename.c_str());

		SetCreate3DSoundFlag(FALSE);// 3Dサウンドフラグを元に戻す
	}

	// 複製ハンドルを使って再生チェック
	if(CheckSoundMem(instance.handle) == 0)
	{
		PlaySoundMem(instance.handle, DX_PLAYTYPE_LOOP);
	}

	Apply3D(instance, pos);
}

void SoundServer3D::StopSound3D(void* key)
{
	auto it = _instance.find(key);
	if(it == _instance.end())
	{
		return;
	}

	// ハンドルを停止・削除
	if(it->second.handle != -1)
	{
		StopSoundMem(it->second.handle);
		DeleteSoundMem(it->second.handle);
	}

	_instance.erase(it);
}

void SoundServer3D::StopAll()
{
	for(auto& [key, instance] : _instance)
	{
		if(instance.handle != -1)
		{
			StopSoundMem(instance.handle);
			DeleteSoundMem(instance.handle);
		}
	}
	_instance.clear();
}


void SoundServer3D::Apply3D(VoiceInstance& instance, const vec::Vec3& pos) const
{
	if(instance.handle == -1)
	{
		return;
	}

	Set3DRadiusSoundMem(instance.radius, instance.handle);// 3D音源の半径を設定
	Set3DPositionSoundMem(DxlibConverter::VecToDxLib(pos), instance.handle);// 3D音源の位置を設定
}