
#include "playertanuki.h"
#include "appframe.h"
#include "applicationglobal.h"


bool PlayerTanuki::Initialize()
{
	if(!base::Initialize()) { return false; }
	LoadModel(mv1::SK_goepon_multimotion);
	_iAttachIndex = -1;

	_status = STATUS::NONE;

	_fTotalTime = 0.0f;
	_fPlayTime = 0.0f;

	_fColSubY = 40.0f;
	
	_fCollisionR = 30.0f;			// キャラの当たり判定用の円の半径
	_fCollisionWeight = 20.0f;		// キャラの重さ
	_cam = nullptr;
	_fMvSpeed = 10.0f;

	_bLand = true;

	return true;
}


bool PlayerTanuki::Terminate()
{
	base::Terminate();

	return true;
}

bool PlayerTanuki::SoundWalk()
{
	if(gGlobal._soundServer)
	{
		auto sound = gGlobal._soundServer->Get("1");
		if(sound)
		{
			if(_status == STATUS::WALK)
			{
				if(!sound->IsPlay())
				{
					sound->Play();
				}
			}
			else
			{
				if(sound->IsPlay())
				{
					sound->Stop();
				}
			}
		}
	}

	return true;
}


bool PlayerTanuki::Process()
{
	base::Process();

	int key = ApplicationBase::GetInstance()->GetKey();


	_vOldPos = _vPos;

	CharaBase::STATUS old_status = _status;

	_v = { 0,0,0 };

	float sx = _cam->_vPos.x - _cam->_vTarget.x;
	float sz = _cam->_vPos.z - _cam->_vTarget.z;
	float camrad = atan2(sz, sx);


	lStickX = fLx;
	lStickZ = fLz;

	vec::Vec3 inputLocal = vec3::VGet(0.0f, 0.0f, 0.0f);
	if(key & PAD_INPUT_DOWN)
	{
		inputLocal.z = 1.0f;
	}
	if(key & PAD_INPUT_UP)
	{
		inputLocal.z = -1.0f;
	}
	if(key & PAD_INPUT_LEFT)
	{
		inputLocal.x = -1.0f;
	}
	if(key & PAD_INPUT_RIGHT)
	{
		inputLocal.x = 1.0f;
	}

	// 左スティックの長さを計算
	float length = sqrtf(lStickX * lStickX + lStickZ * lStickZ);

	// デッドゾーン未満なら十字キーを採用（合成ではなく置き換え）
	if (length < _fAnalogDeadZone)
	{
		// 十字キーの入力を fLx/fLz と同じ軸系に寄せる
		lStickX = inputLocal.x;
		lStickZ = inputLocal.z;

		length = sqrtf(lStickX * lStickX + lStickZ * lStickZ); 
	}

	// ローカル角度
	float localRad = 0.0f;

	if (length >= _fAnalogDeadZone)
	{
		float moveX = lStickZ; // 前後
		float moveZ = lStickX; // 左右

		// 入力ベクトル保存
		_vInput = vec3::VGet(moveX, 0.0f, moveZ);
		// 正規化
		if (vec3::VSize(_vInput) > 0.0f)
		{
			_vInput = vec3::VNorm(_vInput); 
		}

		// 角度は atan2(z, x) がXZ平面の標準
		localRad = atan2f(moveZ, moveX);

		// 速度：アナログは length(0..1想定) * _fMvSpeed にした方が自然
		float speed = _fMvSpeed;
		float moveLen = speed * length;

		// ワールド方向の移動ベクトルに変換
		_v.x = cosf(localRad + camrad) * moveLen;
		_v.z = sinf(localRad + camrad) * moveLen;

		_vDir = _v;
		_status = STATUS::WALK;
	}
	else
	{
		_v = { 0.0f, 0.0f, 0.0f };
		_vInput = vec3::VGet(0.0f, 0.0f, 0.0f);
		_status = STATUS::WAIT;
	}

	if(_fPlayTime >= _fTotalTime)
	{
		_fPlayTime = 0.0f;
	}

	// アニメーションの名前取得
	auto GetAnimName = [this](STATUS name) -> std::string
		{
			switch(name)
			{
			case STATUS::WAIT:
				return "goepon_idle";
			case STATUS::WALK:
				return "goepon_walk";
			default:
				return std::string();
			}
		};

	auto PlayAnim = [&](bool change)
		{
			std::string name = GetAnimName(_status);
			if(name.empty()) { return; }

			_animId = AnimationManager::GetInstance()->Play(_handle, name, true);
			_fPlayTime = 0.0f;

			if(change)
			{
				switch(_status)
				{
				case STATUS::WAIT:
					_fPlayTime += rand() % 30;
					break;
				}
			}

			if(_animId != -1)
			{
				AnimationManager::GetInstance()->SetTime(_animId, _fPlayTime);
			}
		};

	if(_animId != -1 && !AnimationManager::GetInstance()->IsPlaying(_animId))
	{
		_animId = -1;
		PlayAnim(false);
	}

	if(old_status == _status)
	{
		float anim_speed = 0.5f;
		_fPlayTime += anim_speed;
		switch(_status)
		{
		case STATUS::WAIT:
			_fPlayTime += (float)(rand() % 10) / 100.0f;
			break;
		}
	}
	else
	{
		if(_animId != -1)
		{
			AnimationManager::GetInstance()->Stop(_animId);
			_animId = -1;
		}
		PlayAnim(true);
		SoundWalk();
	}

	if(_fPlayTime >= _fTotalTime)
	{
		_fPlayTime = 0.0f;
	}

	SoundWalk();

	return true;
}

bool PlayerTanuki::Render()
{
	base::Render();

	float vorty = atan2(_vDir.x * -1, _vDir.z * -1);// モデルが標準でどちらを向いているかで式が変わる(これは-zを向いている場合)

	MATRIX mRotY = MGetRotY(vorty);

	MATRIX mRotZ = MGetRotZ(DX_PI_F * 0.5f); 

	MATRIX mTrans = MGetTranslate(DxlibConverter::VecToDxLib(_vPos)); // 平行移動行列

	MATRIX mScale = MGetScale(VGet(1.0f, 1.0f, 1.0f));					// 拡大行列

	MATRIX m = MGetIdent();

	//m = MMult(m, mRotZ);
	m = MMult(m, mRotY);
	m = MMult(m, mScale);
	m = MMult(m, mTrans);

	MV1SetMatrix(_handle, m);

	MV1DrawModel(_handle);

	return true;
}