#include "Makimono.h"

bool Makimono::Initialize()
{
	base::Initialize();
	
	_handle = EffekseerManager::GetInstance()->LoadEffect(ef::IT_makimono, 1.0f);
	if(_handle < 0) { return false; }

	_vPos = vec::Vec3{ 0.0f, 0.0f, 0.0f };
	_vDir = vec::Vec3{ 0.0f, 0.0f, -1.0f };
	haveMakimono = false;
	_isVisible = true;
	_drawSize = 60.0f;
	_playHandle = -1;
	return true;
}

bool Makimono::Terminate()
{
	base::Terminate();

	auto em = EffekseerManager::GetInstance();
	if(em && _playHandle != -1)
	{
		em->StopEffect(_playHandle);
		_playHandle = -1;
	}

	if(em && _handle)
	{
		em->DeleteEffect(_handle);
		_handle = -1;
	}

	haveMakimono = false;
	_isVisible = false;
	return true;
}

bool Makimono::Process()
{
	base::Process();
	auto em = EffekseerManager::GetInstance();
	if(!em || _handle == -1) { return true; }

	if(_isVisible)
	{
		// ハンドルが無い、または再生終了していたら再生しなおす = ループ
		if(_playHandle == -1 || !em->IsPlayingEffect(_playHandle))
		{
			_playHandle = em->PlayEffect3DPos(_handle, _vPos);
		}
		else
		{
			em->SetPosEffect(_playHandle, _vPos);
		}
	}
	else
	{
		if(em->IsPlayingEffect(_playHandle))
		{
			em->StopEffect(_playHandle);
		}
		_playHandle = -1;
	}
	return true;
}

bool Makimono::Render()
{
	base::Render();
	return true;
}
