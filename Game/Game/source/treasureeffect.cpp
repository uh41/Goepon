#include "treasureeffect.h"
#include "Treasure.h"

TreasureEffect::TreasureEffect()
{
	Initialize();
}

bool TreasureEffect::Initialize()
{
	base::Initialize();
	_handle = EffekseerManager::GetInstance()->LoadEffect(ef::EF_Tresure, 1.0f);
	_playHandle = -1;

	return true;
}

bool TreasureEffect::Terminate()
{
	base::Terminate();

	auto em = EffekseerManager::GetInstance();
	if(em)
	{
		for(auto& kv : _playHandles)
		{
			const int playHandle = kv.second;
			if(playHandle != -1)
			{
				em->StopEffect(playHandle);
			}
		}
	}
	_playHandles.clear();


	return true;
}

bool TreasureEffect::Process()
{
	base::Process();

	auto em = EffekseerManager::GetInstance();
	if(!em) { return true; }

	// コンテナが空なら、残っている再生を止める
	if(_treasure.empty())
	{
		for(auto& kv : _playHandles)
		{
			const int playHandle = kv.second;
			if(playHandle != -1)
			{
				em->StopEffect(playHandle);
			}
		}
		_playHandles.clear();
		return true;
	}

	// 現在の宝箱をループして、再生/追従/停止
	for(const auto& t : _treasure)
	{
		Treasure* treasure = t.get();
		if(!treasure) { continue; }

		// mapにキーがなければ作られ、初期値は0になる可能性があるため-1に寄せる
		int& playHandle = _playHandles[treasure];
		if(playHandle == 0)
		{
			playHandle = -1;
		}
		if(treasure->IsVisible())
		{
			if(_handle != -1 && playHandle == -1)
			{
				playHandle = em->PlayEffect3DPos(_handle, treasure->GetPos());
			}
			else if(playHandle != -1)
			{
				em->SetPosEffect(playHandle, treasure->GetPos());
			}
		}
		else
		{
			if(playHandle != -1)
			{
				em->StopEffect(playHandle);
				playHandle = -1;
			}
		}
	}

	// コンテナから消えた宝箱の再生ハンドルを停止して削除
	for(auto it = _playHandles.begin(); it != _playHandles.end();)
	{
		Treasure* key = it->first;

		bool stillExists = false;
		for(const auto& t : _treasure)
		{
			// コンテナに存在しているかチェック
			if(t.get() == key)
			{
				stillExists = true; // 存在している
				break;
			}
		}

		// 存在していなければ停止して削除
		if(!stillExists)
		{
			const int playHandle = it->second;
			if(playHandle != -1)
			{
				em->StopEffect(playHandle);
			}
			it = _playHandles.erase(it);
		}
		// 存在していれば次へ
		else
		{
			++it;
		}
	}

	return true;
}

bool TreasureEffect::Render()
{
	base::Render();
	return true;
}