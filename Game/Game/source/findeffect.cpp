#include "findeffect.h"
#include "enemybase.h"
#include "enemysensor.h"

FindEffect::FindEffect()
{
	Initialize();
}

bool FindEffect::Initialize()
{
	base::Initialize();
	// リソース読み込み（Resources.h の ef::EF_find を想定）
	_handle = EffekseerManager::GetInstance()->LoadEffect(ef::EF_find, 1.0f);
	_enemy = at::vspc<EnemyBase>();
	_playHandle = -1;
	_playHandleMap.clear();
	return true;
}

bool FindEffect::Terminate()
{
	base::Terminate();
	StopPlaying();
	return true;
}

void FindEffect::PlayOnce(EnemyBase* enemy)
{
	if(!enemy) return;

	auto em = EffekseerManager::GetInstance();
	if(!em || _handle == -1) return;

	// 敵の座標を取得
	vec::Vec3 pos = enemy->GetPos();

	// その場に1回だけ再生する
	em->PlayEffect3DPos(_handle, pos);
}

bool FindEffect::Process()
{
	base::Process();

	
	return true;
}

bool FindEffect::StopPlaying()
{
	auto em = EffekseerManager::GetInstance();
	if(em)
	{
		// マップ内の全ハンドルを停止
		for(auto&& phm : _playHandleMap)
		{
			if(phm.second != -1)
			{
				em->StopEffect(phm.second);
			}
		}
	}
	_playHandleMap.clear();

	// base クラスの単一ハンドルも念のため停止
	if(_playHandle != -1 && em)
	{
		em->StopEffect(_playHandle);
		_playHandle = -1;
	}
	return true;
}

bool FindEffect::Render()
{
	base::Render();
	return true;
}