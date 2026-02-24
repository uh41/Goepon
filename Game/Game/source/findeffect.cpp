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

bool FindEffect::Process()
{
	base::Process();

	auto em = EffekseerManager::GetInstance();
	if(_handle == -1 || !em) return true;

	// 敵の重複を排除するためのセット
	std::unordered_set<EnemyBase*> enemySet;
	for(auto&& enemy : _enemy)
	{
		if(enemy)
		{
			enemySet.insert(enemy.get());// 重複排除のためセットに追加
		}
	}

	at::vet<EnemyBase*> remove;
	for(auto&& phm : _playHandleMap)
	{
		if(enemySet.find(phm.first) == enemySet.end())
		{
			if(phm.second != -1)
			{
				em->StopEffect(phm.second);
			}
			remove.push_back(phm.first);
		}
	}

	// マップから削除
	for(auto&& rem : remove)
	{
		_playHandleMap.erase(rem);
	}

	// _enemy は at::vspc<EnemyBase>（std::vector<std::shared_ptr<EnemyBase>>）なので、要素をループしてアクセスする必要があります
	for(const auto& enemyPtr : _enemy)
	{
		if(!enemyPtr) continue;
		auto sensor = enemyPtr->GetEnemySensor();
		bool detected = false;
		if(sensor)
		{
			// 検出／追跡中を検知する条件。必要なら他の条件に差し替え可能。
			detected = sensor->IsChasing();
		}

		// 表示位置：敵の位置 + 腰高さ + オフセット（頭上）
		vec::Vec3 pos = enemyPtr->GetPos();

		auto it = _playHandleMap.find(enemyPtr.get());

		if(detected)
		{
			if(_playHandle == -1 || it == _playHandleMap.end() || it->second == -1)
			{
				_playHandle = em->PlayEffect3DPos(_handle, pos);
				_playHandleMap[enemyPtr.get()] = _playHandle;
			}
			else
			{
				em->SetPosEffect(it->second, pos);
			}
		}
		else
		{
			if(_playHandle != -1 && it != _playHandleMap.end() && it->second != -1)
			{
				em->StopEffect(it->second);
				_playHandleMap.erase(it);
			}
		}
	}
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