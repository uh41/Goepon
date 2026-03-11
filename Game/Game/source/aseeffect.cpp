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

	// タヌキのクールダウン中のみ汗エフェクトを出す
	bool needEffect = false;
	PlayerTanuki* tan = dynamic_cast<PlayerTanuki*>(_player);

	// タヌキプレイヤーである場合のみエフェクトを出す（クールダウン時間で判定）
	if(tan != nullptr)
	{
		if(tan->GetDashCoolDownTime() > 0.0f)
		{
			needEffect = true;
		}
	}
	else
	{
		// タヌキではない場合、エフェクトを確実に停止
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

	// needEffect が false の場合は再生中のエフェクトを停止して早期リターン
	if(!needEffect)
	{
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
	else
	{
		// 向き情報が無ければ再生しない
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

	// 左側ベクトル
	vec::Vec3 leftDir = vec::Vec3(-forward.z, 0.0f, forward.x);

	// エフェクト位置計算
	const float headOffsetY = 100.0f; // 高さ
	const float leftOffsetAmt = 40.0f; // 左への距離
	vec::Vec3 effectPos = _player->GetPos();
	effectPos.y += headOffsetY;
	effectPos.x += leftDir.x * leftOffsetAmt;
	effectPos.z += leftDir.z * leftOffsetAmt;

	// 回転は常にプレイヤーの向きから算出して設定する（左右限定をやめる）
	float yaw = atan2(playerDir.x, playerDir.z);
	vec::Vec3 rot = vec::Vec3(0.0f, yaw, 0.0f);

	// 再生ハンドルが無ければ新規再生
	if(_playHandle == -1)
	{
		_playHandle = em->PlayEffect3DPos(_handle, effectPos);
		if(_playHandle != -1)
		{
			em->SetRotationEffect(_playHandle, rot);
		}
	}
	else
	{
		// ハンドルがある場合は再生中なら位置更新、再生終了なら再生再開
		if(em->IsPlayingEffect(_playHandle))
		{
			em->SetPosEffect(_playHandle, effectPos);
			em->SetRotationEffect(_playHandle, rot);
		}
		else
		{
			_playHandle = em->PlayEffect3DPos(_handle, effectPos);
			if(_playHandle != -1)
			{
				em->SetRotationEffect(_playHandle, rot);
			}
		}
	}

	return true;
}

bool AseEffect::Render()
{
	base::Render();
	return true;
}