#include "walkeffect.h"

WalkEffect::WalkEffect()
{
	Initialize();
}

WalkEffect::~WalkEffect()
{
	
}

bool WalkEffect::Initialize()
{
	base::Initialize();
	_handle = EffekseerManager::GetInstance()->LoadEffect(ef::EF_walk, 1.0f);
	_chara = nullptr;
	_playHandle = -1;
	return true;
}

bool WalkEffect::Terminate()
{
	base::Terminate();
	if(_playHandle != -1)
	{
		auto em = EffekseerManager::GetInstance();
		if(em)
		{
			em->StopEffect(_playHandle);
			_playHandle = -1;
		}
	}

	return true;
}

bool WalkEffect::Process()
{
	base::Process();
	auto em = EffekseerManager::GetInstance();
	if(!_chara)
	{
		return true;
	}

	vec::Vec3 charaPos = _chara->GetPos();
	// プレイヤーが歩行中のときのみエフェクトを再生／更新する
	if(_chara->_status == CharaBase::STATUS::WALK)
	{
		if(_handle != -1)
		{
			if(_playHandle == -1)
			{
				// 再生開始（初回）
				_playHandle = em->PlayEffect3DPos(_handle, charaPos);
			}
			else
			{
				// 既に再生中なら位置を毎フレーム更新して追従させる
				em->SetPosEffect(_playHandle, charaPos);
			}
		}
	}
	else
	{
		// 止まっている（歩行でない）なら再生中のエフェクトがあれば停止する
		if(_playHandle != -1)
		{
			if(em)
			{
				em->StopEffect(_playHandle);
			}
		}
	}
	return true;
}

bool WalkEffect::Render()
{
	base::Render();
	return true;
}
