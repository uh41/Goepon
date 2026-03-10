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

	// XZ平面の前方ベクトルを正規化（上下成分は無視）
	vec::Vec3 forward = playerDir;
	forward.y = 0.0f;
	if(vec3::VSize(forward) > 0.0f)
	{
		forward = vec3::VNorm(forward);
	}

	// 2. 「左側」の方向ベクトルを計算 (常にプレイヤーの真左を指す)
	// Y軸(0,1,0)と前方の外積で「左」を求める
	vec::Vec3 leftDir = vec::Vec3(-forward.z, 0.0f, forward.x);

	// 3. 座標の計算
	const float headOffsetY = 100.0f; // 高さ
	const float leftOffsetAmt = 40.0f; // 左への距離
	vec::Vec3 effectPos = _player->GetPos();
	effectPos.y += headOffsetY;
	effectPos.x += leftDir.x * leftOffsetAmt;
	effectPos.z += leftDir.z * leftOffsetAmt;

	// 4. 回転の計算 (ここが重要！)
	// プレイヤーの向きを判定（X軸方向の成分が大きいなら左右を向いている）
	float absX = fabsf(forward.x);
	float absZ = fabsf(forward.z);

	vec::Vec3 rotation = vec::Vec3(0.0f, 0.0f, 0.0f);

	if(absX > absZ)
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
			_playHandle = em->PlayEffect3DPos(_handle, effectPos);
			if(_playHandle != -1)
			{
				em->SetRotationEffect(_playHandle, rotation);
			}
		}
		else
		{
			// ハンドルがある場合は再生中なら位置更新、再生終了なら再生再開
			if(em->IsPlayingEffect(_playHandle))
			{
				em->SetPosEffect(_playHandle, effectPos);
				em->SetRotationEffect(_playHandle, rotation);
			}
			else
			{
				_playHandle = em->PlayEffect3DPos(_handle, effectPos);
				if(_playHandle != -1)
				{
					em->SetRotationEffect(_playHandle, rotation);
				}
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