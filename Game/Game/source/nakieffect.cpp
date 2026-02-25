#include "nakieffect.h"

NakiEffect::NakiEffect()
{
	Initialize();
}

bool NakiEffect::Initialize()
{
	base::Initialize();
	_handle = EffekseerManager::GetInstance()->LoadEffect(ef::EF_naki, 1.0f);
	_isPlay = false;
	_playHandle = -1;
	_targetPlayer = nullptr;
	return true;
}

bool NakiEffect::Terminate()
{
	base::Terminate();
	auto em = EffekseerManager::GetInstance();
	if(em && _handle != -1)
	{
		em->DeleteEffect(_handle);
		_handle = -1;
	}
	StopPlaying();
	_targetPlayer = nullptr;
	return true;
}

void NakiEffect::PlayEffect(const vec::Vec3& pos)
{
	// 既に一度再生している場合は再生しない
	if(_isPlay)
	{
		return;
	}

	auto em = EffekseerManager::GetInstance();
	if(!em || _handle == -1)
	{
		return;
	}

	// 再生インスタンスのハンドルを保存する
	int playHandle = em->PlayEffect3DPos(_handle, pos);
	if(playHandle != -1)
	{
		_playHandle = playHandle;
		_isPlay = true;

		// 追従対象が設定されていれば初期位置を合わせる
		if(_targetPlayer)
		{
			em->SetPosEffect(_playHandle, _targetPlayer->GetPos());
		}
	}
}

void NakiEffect::ResetEffect()
{
	StopPlaying();
	_isPlay = false;
	_playHandle = -1;
}

bool NakiEffect::Process()
{
	base::Process();

	// 再生中で追従対象があれば毎フレーム位置を更新
	if(_isPlay && _playHandle != -1 && _targetPlayer)
	{
		auto em = EffekseerManager::GetInstance();
		if(em)
		{
			em->SetPosEffect(_playHandle, _targetPlayer->GetPos());
		}
	}

	return true;
}

bool NakiEffect::Render()
{
	base::Render();
	return true;
}