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

	// 宝箱が一つもないなら全て停止して終了
	if(_treasureBase.empty())
	{
		// 全て停止
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

	// 各宝箱ごとに処理
	for(const auto& t : _treasureBase)
	{
		TreasureBase* treasure = t.get();
		if(!treasure) { continue; }

		// 存在しなければ-1で初期化
		auto it = _playHandles.find(treasure);
		if(it == _playHandles.end())
		{
			// 新規追加後のイテレータ取得
			// emplace = コンテナ内に要素を直接構築する
			it = _playHandles.emplace(treasure, -1).first; 
		}
		int& playHandle = it->second; // 再生ハンドル参照

		// 宝箱が空いているなら止める
		if(treasure->IsOpen())
		{
			if(playHandle != -1)
			{
				em->StopEffect(playHandle);
				playHandle = -1;
			}
			continue;
		}

		// 宝箱が表示されているならエフェクト再生
		if(treasure->IsVisible())
		{
			if(_handle != -1 && playHandle == -1)
			{
				playHandle = em->PlayEffect3DPos(_handle, treasure->GetPos());	// 位置指定で再生
			}
			else if(playHandle != -1)
			{
				em->SetPosEffect(playHandle, treasure->GetPos()); // 位置更新
			}
		}
		else
		{
			if(playHandle != -1)
			{
				em->StopEffect(playHandle);	 // 非表示なら止める
				playHandle = -1;
			}
		}
	}

	// _treasure に存在しない宝箱のハンドルを削除
	for(auto it = _playHandles.begin(); it != _playHandles.end();)
	{
		TreasureBase* key = it->first;

		bool isInTreasureList = false; // _treasure に存在するか
		for(const auto& t : _treasureBase)
		{
			if(t.get() == key)
			{
				isInTreasureList = true;
				break;
			}
		}
		if(!isInTreasureList)
		{
			const int playHandle = it->second;
			if(playHandle != -1)
			{
				em->StopEffect(playHandle);
			}
			it = _playHandles.erase(it); // 存在しないなら削除
		}
		else
		{
			++it;
		}
	}
	
	return true;
}

void TreasureEffect::StopEffect(TreasureBase* treasure)
{
	if(!treasure) return;
	auto em = EffekseerManager::GetInstance();
	if(!em) return;

	auto it = _playHandles.find(treasure);
	if(it == _playHandles.end()) return;

	int& playHandle = it->second;
	if(playHandle != -1)
	{
		em->StopEffect(playHandle);
		playHandle = -1;
	}
	// マップから消す（安全策）
	_playHandles.erase(it);
}

bool TreasureEffect::Render()
{
	base::Render();
	return true;
}