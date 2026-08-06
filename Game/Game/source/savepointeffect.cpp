#include "savepointeffect.h"
#include "savepoint.h"
#include "playerform.h"

SavePointEffect::SavePointEffect()
{
	_savePointEffectHandle.clear();
	_savePointEffectIsKirakira.clear();
	_efSaveHandle = -1;
	_efKirakiraHandle = -1;
	_targetPlayer = nullptr;
	Initialize();
}

bool SavePointEffect::Initialize()
{
	base::Initialize();
	auto em = EffekseerManager::GetInstance();
	if(em)
	{
		_efSaveHandle = em->LoadEffect(ef::Savepoint);
		_efKirakiraHandle = em->LoadEffect(ef::EF_Kirakira);
	}
	_playHandle = -1;
	return true;
}

bool SavePointEffect::Terminate()
{
	base::Terminate();
	auto em = EffekseerManager::GetInstance();
	if(em)
	{
		// 停止してからプレイヤブルエフェクトインスタンスをクリア
		for(auto& kv : _savePointEffectHandle)
		{
			const int playHandle = kv.second;
			if(playHandle != -1)
			{
				em->StopEffect(playHandle);
			}
		}

		// ロード済みのエフェクトは削除
		if(_efSaveHandle != -1)
		{
			em->DeleteEffect(_efSaveHandle);
			_efSaveHandle = -1;
		}
		if(_efKirakiraHandle != -1)
		{
			em->DeleteEffect(_efKirakiraHandle);
			_efKirakiraHandle = -1;
		}
	}
	_savePointEffectHandle.clear();
	_savePointEffectIsKirakira.clear();
	return true;
}

bool SavePointEffect::Process()
{
	base::Process();

	auto em = EffekseerManager::GetInstance();
	if(!em) { return true; }

	if(!_targetPlayer)
	{
		auto playerForm = PlayerForm::GetInstance();
		if(playerForm)
		{
			_targetPlayer = playerForm->GetPlayer();
		}
	}

	if(!_targetPlayer)
	{
		return true;
	}

	// 一括停止用ラムダ
	auto stopHandle = [&](int& h) {
		if(h != -1) {
			em->StopEffect(h);
			h = -1;
		}
		};

	// SavePoint が無ければ全停止
	if(_savePoint.empty())
	{
		for(auto& kv : _savePointEffectHandle)
		{
			stopHandle(kv.second);
		}
		_savePointEffectHandle.clear();
		_savePointEffectIsKirakira.clear();
		return true;
	}

	std::unordered_set<SavePoint*> current;
	current.reserve(_savePoint.size());

	for(const auto& spv : _savePoint)
	{
		SavePoint* sp = spv.get();
		if(!sp) continue;
		current.insert(sp);

		// マップにエントリが無ければ追加
		auto it = _savePointEffectHandle.find(sp);
		if(it == _savePointEffectHandle.end())
		{
			it = _savePointEffectHandle.emplace(sp, -1).first;
		}
		int& playHandle = it->second;

		// 初期種別フラグ（false = Savepoint）
		auto itType = _savePointEffectIsKirakira.find(sp);
		if(itType == _savePointEffectIsKirakira.end())
		{
			_savePointEffectIsKirakira.emplace(sp, false);
			itType = _savePointEffectIsKirakira.find(sp);
		}
		bool& isKirakiraPlaying = itType->second;

		// モデルが有効なら位置更新／当たり判定
		if(sp->GetHandle() != -1)
		{
			// MV1行列を更新してコリジョン情報を更新
			int h = sp->GetHandle();
			int f = sp->GetSavePointCollisionFrame();
			MATRIX model = sp->MakeModelMatrix();
			MV1SetMatrix(h, model);
			MV1RefreshCollInfo(h, f);

			// プレイヤーとの接触判定（ターゲットプレイヤーは EffectBase::_targetPlayer を使用）
			bool touching = false;
			if(_targetPlayer)
			{
				vec::Vec3 hitPos;
				if(CollisionManager::GetInstance()->CheckPositionToMV1Collision(
					_targetPlayer->GetPos(),
					h,
					f,
					_targetPlayer->GetColSubY(),
					hitPos
				))
				{
					touching = true;
				}
			}

			// 再生すべきエフェクトを決定
			int desiredEfHandle;
			if(touching)
			{
				desiredEfHandle = _efKirakiraHandle;
			}
			else
			{
				desiredEfHandle = _efSaveHandle;
			}
			bool desiredIsKirakira = touching;

			// まだ再生ハンドルが無ければ再生、既にある場合は種類が一致するなら位置更新、違えば差し替え
			if(playHandle == -1)
			{
				if(desiredEfHandle != -1)
				{
					playHandle = em->PlayEffect3DPos(desiredEfHandle, sp->GetPos());
					isKirakiraPlaying = desiredIsKirakira;
				}
			}
			else
			{
				// 既に再生中だが種類が変わっていたら差し替え
				if(isKirakiraPlaying != desiredIsKirakira)
				{
					stopHandle(playHandle);
					if(desiredEfHandle != -1)
					{
						playHandle = em->PlayEffect3DPos(desiredEfHandle, sp->GetPos());
						isKirakiraPlaying = desiredIsKirakira;
					}
				}
				else
				{
					// 同じ種類なら位置更新
					em->SetPosEffect(playHandle, sp->GetPos());
				}
			}
		}
		else
		{
			// モデル負荷されていないなら停止
			stopHandle(playHandle);
			_savePointEffectIsKirakira.erase(sp);
		}
	}

	// マップに無くなったもののクリーンアップ
	for(auto it2 = _savePointEffectHandle.begin(); it2 != _savePointEffectHandle.end(); )
	{
		if(current.find(it2->first) == current.end())
		{
			stopHandle(it2->second);
			_savePointEffectIsKirakira.erase(it2->first);
			it2 = _savePointEffectHandle.erase(it2);
		}
		else
		{
			++it2;
		}
	}

	return true;
}

bool SavePointEffect::Render()
{
	base::Render();
	return true;
}