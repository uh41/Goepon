#include "savepointeffect.h"
#include "savepoint.h"

SavePointEffect::SavePointEffect()
{
	_savePointEffectHandle.clear();
	Initialize();
}

bool SavePointEffect::Initialize()
{
	base::Initialize();
	_handle = EffekseerManager::GetInstance()->LoadEffect(ef::Savepoint);
	_playHandle = -1;
	return true;
}

bool SavePointEffect::Terminate()
{
	base::Terminate();
	auto em = EffekseerManager::GetInstance();
	if(em)
	{
		for(auto& kv : _savePointEffectHandle)
		{
			const int playHandle = kv.second;
			if(playHandle != -1)
			{
				em->StopEffect(playHandle);
			}
		}
	}
	_savePointEffectHandle.clear();
	return true;
}

bool SavePointEffect::Process()
{
	base::Process();

	auto em = EffekseerManager::GetInstance();
	if(!em) { return true; }

	// ハンドル停止のユーティリティ
	auto stopHandle = [&](int& h) {
		if(h != -1) {
			em->StopEffect(h);
			h = -1;
		}
		};

	// SavePoint が空なら全停止して終了
	if(_savePoint.empty())
	{
		for(auto& kv : _savePointEffectHandle)
		{
			stopHandle(kv.second);
		}
		_savePointEffectHandle.clear();
		return true;
	}

	// 現在の SavePoint ポインタ集合を作る
	std::unordered_set<SavePoint*> current;
	current.reserve(_savePoint.size());
	for(const auto& spv : _savePoint)
	{
		SavePoint* sp = spv.get();
		if(!sp) continue;
		current.insert(sp);

		// マップにエントリがなければ作成
		auto it = _savePointEffectHandle.find(sp);
		if(it == _savePointEffectHandle.end())
		{
			it = _savePointEffectHandle.emplace(sp, -1).first;
		}
		int& playHandle = it->second;

		// モデルハンドルが有効なら再生／位置更新、無効なら停止
		if(sp->GetHandle() != -1)
		{
			if(_handle != -1 && playHandle == -1)
			{
				playHandle = em->PlayEffect3DPos(_handle, sp->GetPos());
			}
			else if(playHandle != -1)
			{
				em->SetPosEffect(playHandle, sp->GetPos());
			}
		}
		else
		{
			stopHandle(playHandle);
		}
	}

	// マップ内で現在リストにないものを削除（停止して消す）
	for(auto it = _savePointEffectHandle.begin(); it != _savePointEffectHandle.end(); )
	{
		if(current.find(it->first) == current.end())
		{
			stopHandle(it->second);
			it = _savePointEffectHandle.erase(it);
		}
		else
		{
			++it;
		}
	}

	return true;
}

bool SavePointEffect::Render()
{
	base::Render();
	return true;
}