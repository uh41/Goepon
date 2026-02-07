#include "aseeffect.h"
#include "enemysensor.h"

AseEffect::AseEffect()
{
	Initialize();
}

bool AseEffect::Initialize()
{
	base::Initialize();
	_handle = EffekseerManager::GetInstance()->LoadEffect(ef::EF_ase, 1.0f);
	_player = nullptr;
	_enemy = at::vspc<EnemyBase>();
	_playHandle = -1;
	return true;
}

bool AseEffect::Terminate()
{
	base::Terminate();
	auto em = EffekseerManager::GetInstance();

	// 再生中なら停止してハンドルクリア
	if(em && _playHandle != -1)
	{
		if(em->IsPlayingEffect(_playHandle))
		{
			em->StopEffect(_playHandle);
		}
		_playHandle = -1;
	}


	if(em && _handle != -1)
	{
		em->DeleteEffect(_handle);
		_handle = -1;
	}
	return true;
}

bool AseEffect::Process()
{
	base::Process();

	auto em = EffekseerManager::GetInstance();
	if(!_player || _handle == -1 || !em)
	{
		return true;
	}

	bool chasing = false;
	for(auto&& enemy : _enemy)
	{
		if(!enemy)
		{
			continue;
		}

		auto sensor = enemy->GetEnemySensor();
		if((sensor && sensor->IsChasing()) || enemy->IsDetectPlayer())
		{
			chasing = true;
			break;
		}
	}

	vec::Vec3 playerPos = _player->GetPos();

	if(chasing)
	{
		// 再生ハンドルが無ければ新規再生
		if(_playHandle == -1)
		{
			_playHandle = em->PlayEffect3DPos(_handle, playerPos);
		}
		else
		{
			// ハンドルがある場合は再生中なら位置更新、再生終了なら再生し直す
			if(em->IsPlayingEffect(_playHandle))
			{
				em->SetPosEffect(_playHandle, playerPos);
			}
			else
			{
				_playHandle = em->PlayEffect3DPos(_handle, playerPos);
			}
		}
	}
	else
	{
		// 追跡が無ければ確実に停止（ハンドル有効時のみ）
		if(_playHandle != -1)
		{
			if(em->IsPlayingEffect(_playHandle))
			{
				em->StopEffect(_playHandle);
			}
			_playHandle = -1;
		}
	}

	return true;
}

bool AseEffect::Render()
{
	base::Render();
	return true;
}
