#include "playermono.h"
#include "applicationglobal.h"

PlayerMono::PlayerMono()
{
	Initialize();
}

PlayerMono::~PlayerMono()
{

}

bool PlayerMono::Initialize()
{
	if(!base::Initialize()) { return false; }
	_handle = MV1LoadModel(mv1::sigaraki);
	_iAttachIndex = -1;

	_status = STATUS::NONE;

	_fTotalTime = 0.0f;
	_fPlayTime = 0.0f;

	_fColSubY = 40.0f;

	_fCollisionR = 30.0f;			// キャラの当たり判定用の円の半径
	_fCollisionWeight = 20.0f;		// キャラの重さ

	_cam = nullptr;
	_fMvSpeed = 4.0f;
	_bLand = true;

	return true;
}

bool PlayerMono::Terminate()
{
	base::Terminate();
	return true;
}

bool PlayerMono::PlayerMonoSoundMove()
{
	if(gGlobal._soundServer)
	{
		auto sound = gGlobal._soundServer->Get("50");
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

bool PlayerMono::Process()
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

	float length = sqrt(lStickX * lStickX + lStickZ * lStickZ);
	float rad = 0.0f;

	if(length >= _fAnalogDeadZone)
	{
		float moveX = lStickZ;
		float moveZ = lStickX;

		_vInput = vec3::VGet(moveX, 0.0f, moveZ);

		if(vec3::VSize(_vInput) > 0.0f)
		{
			_vInput = vec3::VNorm(_vInput);
		}

		rad = atan2(moveZ, moveX);

		_v.x = cos(rad + camrad) * length;
		_v.z = sin(rad + camrad) * length;

		_vDir = _v;
		_status = STATUS::WALK;
	}
	else
	{
		_v = { 0,0,0 };
		_vInput = vec3::VGet(0.0f, 0.0f, 0.0f);
		_status = STATUS::WAIT;
	}

	PlayerMonoSoundMove();

	if(old_status != _status)
	{
		PlayerMonoSoundMove();
	}

	// アニメーションの名前取得
	auto GetAnimName = [this](STATUS name) -> std::string
		{
			switch(name)
			{
			case STATUS::WAIT:
				return "idle_kari";
			case STATUS::WALK:
				return "walk";
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
		float animSpeed = 0.5f;
		_fPlayTime += animSpeed;
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
	}

	if(_fPlayTime >= _fTotalTime)
	{
		_fPlayTime = 0.0f;
	}

	return true;
}

bool PlayerMono::Render()
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

