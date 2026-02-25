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

	// 変更点: PlayEffect3DPos の戻り値（再生インスタンスのハンドル）を保存する
	int playHandle = em->PlayEffect3DPos(_handle, pos);
	if(playHandle != -1)
	{
		_playHandle = playHandle; // リソースハンドルではなく再生ハンドルを保持する
		_isPlay = true; // 再生済みにする
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
	return true;
}

bool NakiEffect::Render()
{
	base::Render();
	return true;
}