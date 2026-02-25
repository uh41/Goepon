
#include "playertanuki.h"
#include "appframe.h"
#include "applicationglobal.h"


bool PlayerTanuki::Initialize()
{
	if(!base::Initialize()) { return false; }
	LoadModel(mv1::SK_goepon_multimotion_4);
	_iAttachIndex = -1;

	_status = STATUS::NONE;

	_fTotalTime = 0.0f;
	_fPlayTime = 0.0f;

	_fColSubY = 40.0f;
	
	_fCollisionR = 30.0f;			// キャラの当たり判定用の円の半径
	_fCollisionWeight = 20.0f;		// キャラの重さ
	_cam = nullptr;
	_fMvSpeed = 8.0f;

	_normalSpeed = _fMvSpeed;
	_dash = false;
	_dashTimer = 0.0f;
	_dashDuration = 3.0f;
	_dashSpeed = 2.0f;// ダッシュ中は通常速度の2倍
	_dashCount = 0;

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
			// パッドの方向キーが押されているか検出
			int key = ApplicationBase::GetInstance()->GetKey();
			bool padPressed = (key & (PAD_INPUT_LEFT | PAD_INPUT_RIGHT | PAD_INPUT_UP | PAD_INPUT_DOWN)) != 0;

			// アナログスティックでの移動判定（しきい値以上なら移動）
			float stickLen = sqrtf(lStickX * lStickX + lStickZ * lStickZ);
			bool analogMove = (stickLen >= _fAnalogDeadZone);

			// 歩行ステータスかつ方向入力（十字キー or アナログ）がある間だけ鳴らす
			if(_status == STATUS::WALK && (padPressed || analogMove))
			{
				// 既に再生中でなければ再生（毎フレーム呼ばれても重複再生しないようにする）
				if(!sound->IsPlay())
				{
					sound->Play();
				}
			}
			else
			{
				// 入力が無ければ再生中なら停止
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
	int trg = ApplicationBase::GetInstance()->GetTrg();

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

		if(!_dash && (trg & PAD_INPUT_2) && _dashCount < dash::DASH_MAX)
		{
			_dash = true;
			_dashTimer = _dashDuration;
			_fMvSpeed = _normalSpeed * _dashSpeed; // ダッシュ開始時に速度を上げる
			_dashCount++;
		}
	}
	else
	{
		_v = { 0.0f, 0.0f, 0.0f };
		_vInput = vec3::VGet(0.0f, 0.0f, 0.0f);
		_status = STATUS::WAIT;
	}

	if(_dash)
	{
		_dashTimer -= 1.0f/ 60.0f; // 仮に60FPSで更新されると想定してタイマーを進める
		if(_dashTimer <= 0.0f)
		{
			_dash = false;
			_dashTimer = 0.0f;
			_fMvSpeed = _normalSpeed;
		}
	}

	if(_fPlayTime >= _fTotalTime)
	{
		_fPlayTime = 0.0f;
	}

	if(old_status != _status)
	{
		SoundWalk();
	}

	// アニメーションの名前取得
	auto GetAnimName = [this](STATUS name) -> std::string
		{
			switch(name)
			{
			case STATUS::WAIT:
				return "idle";
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
		float anim_speed = 0.5f;
		if(_dash && _status == STATUS::WALK)
		{
			anim_speed *= _dashSpeed; // ダッシュ中はアニメーション速度も速くする
		}
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
	}

	if(_fPlayTime >= _fTotalTime)
	{
		_fPlayTime = 0.0f;
	}
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

#ifdef _DEBUG
	// ダッシュ時間をデバッグ表示（現在のタイマーと設定された持続時間）
	{
		unsigned int col = GetColor(255, 255, 0);
		// 表示位置は必要に応じて調整してください
		DrawFormatString(10, 40, col, "Dash timer: %.2f / %.2f", _dashTimer, _dashDuration);
		// ダッシュ残回数も表示
		DrawFormatString(10, 56, col, "Dash used: %d / %d", _dashCount, dash::DASH_MAX);
	}
#endif

	return true;
}