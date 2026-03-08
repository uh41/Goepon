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

	// タヌキのダッシュ時のみ汗エフェクトを出す（敵に見つかっている場合は排除）
	bool needEffect = false;
	PlayerTanuki* tan = dynamic_cast<PlayerTanuki*>(_player);

	// タヌキプレイヤーである場合のみエフェクトを出す
	if(tan != nullptr)
	{
		if(tan->GetDashCoolDownTime() > 0.0f)
		{
			needEffect = true;
		}
	}
	else
	{
		// タヌキではない場合（人間やモノ）、エフェクトを確実に停止
		if(_playHandle != -1)
		{
			if(em->IsPlayingEffect(_playHandle))
			{
				em->StopEffect(_playHandle);
			}
			_playHandle = -1;
		}
		return true;
	}

	vec::Vec3 playerPos = _player->GetPos();
	vec::Vec3 playerDir = _player->GetDir();

	// 頭上に出すためのオフセット計算
	const float headOffsetY = 100.0f;   // Y軸方向の頭上オフセット（調整可能）
	const float forwardOffset = 60.0f;  // 前方オフセット（調整可能）

	// XZ平面の前方ベクトルを正規化（上下方向は無視）
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

		// 右向き（XZ平面で正のX軸側に向かっている）ならエフェクトを180度回転させる
		if(forward.x > 0.5f)
		{
			yaw += PI; // 180度回転
		}

		vec::Vec3 rot = vec::Vec3(0.0f, yaw, 0.0f);
		// 再生ハンドルがない場合は新規再生
		if(_playHandle == -1)
		{
			_playHandle = em->PlayEffect3DPos(_handle, playerPos);
			em->SetRotationEffect(_playHandle, rot);
		}
		else
		{
			// ハンドルがある場合は再生中なら位置更新、再生終了なら再生再開
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
		// 条件を満たさないときは確実に停止（ハンドルが有効のみ）
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