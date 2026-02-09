#include "walkeffect.h"

WalkEffect::WalkEffect()
{
	Initialize();
}

bool WalkEffect::Initialize()
{
	base::Initialize();
	_handle = EffekseerManager::GetInstance()->LoadEffect(ef::EF_walk, 1.0f);
	_playerBase = nullptr;
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
	if(!_playerBase)
	{
		return true;
	}

	vec::Vec3 charaPos = _playerBase->GetPos();
	vec::Vec3 charaDir = _playerBase->GetDir();
	// プレイヤーが歩行中のときのみエフェクトを再生／更新する
	if(_playerBase->_status == CharaBase::STATUS::WALK)
	{
		if(_handle != -1)
		{
			// 既存ハンドルが再生終了しているか確認してクリアする
			if(_playHandle != -1 && !em->IsPlayingEffect(_playHandle))
			{
				// 再生が終わっているのでハンドルをクリアして次フレームで再生可能にする
				_playHandle = -1;
			}

			float yaw = atan2(charaDir.x, charaDir.z);
			vec::Vec3 rot = vec::Vec3(0.0f, yaw, 0.0f);

			if(_playHandle == -1)
			{
				// 再生開始（初回・または終了後の再生）
				_playHandle = em->PlayEffect3DPos(_handle, charaPos);
				em->SetRotationEffect(_playHandle, rot);
			}
			else
			{
				// 既に再生中なら位置を毎フレーム更新して追従させる
				em->SetPosEffect(_playHandle, charaPos);
				em->SetRotationEffect(_playHandle, rot);
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
				_playHandle = -1;
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
