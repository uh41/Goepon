#include "aseeffect.h"
#include "playertanuki.h"

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

	// タヌキのダッシュ時に汗エフェクトを出す（敵に見つかった判定は廃止）
	bool needEffect = false;
	PlayerTanuki* tan = dynamic_cast<PlayerTanuki*>(_player);
	if(tan != nullptr)
	{
		if(tan->GetDashCoolDownTime() > 0.0f)
		{
			needEffect = true;
		}
	}

	vec::Vec3 playerPos = _player->GetPos();
	vec::Vec3 playerDir = _player->GetDir();

	// 頭前方に出すためのオフセット計算
	const float headOffsetY = 100.0f;   // Y方向の頭上オフセット（適宜調整）
	const float forwardOffset = 60.0f;  // 前方オフセット（適宜調整）

	// XZ平面の前方ベクトルを正規化（上下成分は無視）
	vec::Vec3 forward = playerDir;
	forward.y = 0.0f;
	if(vec3::VSize(forward) > 0.0f)
	{
		forward = vec3::VNorm(forward);
	}

	playerPos.x += forward.x * forwardOffset;
	playerPos.z += forward.z * forwardOffset;
	playerPos.y += headOffsetY;

	if(needEffect)
	{
		float yaw = atan2(playerDir.x, playerDir.z);

		// 右向き（XZ平面で正のX成分が強い）ならエフェクトを180度回転させる
		if(forward.x > 0.5f)
		{
			yaw += PI; // 180度回転
		}

		vec::Vec3 rot = vec::Vec3(0.0f, yaw, 0.0f);
		// 再生ハンドルが無ければ新規再生
		if(_playHandle == -1)
		{
			_playHandle = em->PlayEffect3DPos(_handle, playerPos);
			em->SetRotationEffect(_playHandle, rot);
		}
		else
		{
			// ハンドルがある場合は再生中なら位置更新、再生終了なら再生し直す
			if(em->IsPlayingEffect(_playHandle))
			{
				em->SetPosEffect(_playHandle, playerPos);
				em->SetRotationEffect(_playHandle, rot);
			}
			else
			{
				_playHandle = em->PlayEffect3DPos(_handle, playerPos);
				em->SetRotationEffect(_playHandle, rot);
			}
		}
	}
	else
	{
		// 条件が満たされなければ確実に停止（ハンドル有効時のみ）
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