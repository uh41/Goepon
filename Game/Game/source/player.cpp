/*********************************************************************/
// * \file   player.cpp
// * \brief  人狸状態クラス
// *
// * \author 鈴木裕稀
// * \date   2025/12/15
// * \作業内容: 新規作成 鈴木裕稀　2025/12/15
//			  : hp追加	鈴木裕稀 2026/01/06
/*********************************************************************/

#include "player.h"
#include "appframe.h"
#include "applicationglobal.h"

// 初期化
bool Player::Initialize()
{
	if(!base::Initialize()) { return false; }
	_handle = MV1LoadModel(mv1::SK_multimotionTanuHuman_06);
	_iAttachIndex = -1;
	_animId = -1;
	// ステータスを「無し」に設定
	_status = STATUS::NONE;
	// 再生時間の初期化
	_fTotalTime = 0.0f;
	_fPlayTime = 0.0f;
	// 位置、向きの初期化
	_vPos = vec3::VGet(0.0f, 0.0f, 0.0f); // 初期位置が同じだが、押し出され処理のおかげで位置がずれる
	_vDir = vec3::VGet(0.0f, 0.0f, -1.0f);// キャラモデルはデフォルトで-Z方向を向いている
	// 腰位置の設定
	_fColSubY = 40.0f;
	// コリジョン半径の設定
	_fCollisionR = 30.0f;
	_fCollisionWeight = 20.0f;
	_cam = nullptr;
	// 移動速度設定
	_fMvSpeed = 5.0f;

	_iAxisHoldCount = 0;		// 十字キー水平入力保持カウント初期化
	_bAxisUseLock = false;			// 十字キー水平入力ロックフラグ初期化
	_iAxisThreshold = 15;		// 十字キー水平入力保持閾値設定

	_vAxisLockDir = vec3::VGet(0.0f, 0.0f, -1.0f);

	// カメラオフセット初期化
	_camOffset = vec3::VGet(0.0f, 0.0f, 0.0f);
	_camTargetOffset = vec3::VGet(0.0f, 0.0f, 0.0f);

	_bLand = true;

	// 初期体力設定
	_fHp = 20.0f;

	_tailCollisionFrame = MV1SearchFrame(_handle, "tailcolishon");

	if(_tailCollisionFrame >= 0)
	{
		// コリジョン情報生成（必要に応じて分解能を調整）
		MV1SetupCollInfo(_handle, _tailCollisionFrame, 8, 8, 8);
		MV1SetFrameVisible(_handle, _tailCollisionFrame, FALSE);
	}

	_inputDisabled = false; // 入力は初期状態で有効

	_prevAxisUseLock = false;
	_animName.clear();

	return true;
}

// 終了
bool Player::Terminate()
{
	base::Terminate();
	return true;
}

bool Player::PlayerSoundMove()
{
	if(gGlobal._soundServer)
	{
		auto sound = gGlobal._soundServer->Get("11");
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

// 計算処理
bool Player::Process()
{
	int key = ApplicationBase::GetInstance()->GetKey();
	int trg = ApplicationBase::GetInstance()->GetTrg();

	base::Process();

	// 処理前の位置を保存（他の当たり判定で参照するため）
	_vOldPos = _vPos;

	// 処理前のステータスを保存しておく
	CharaBase::STATUS old_status = _status;
	// 移動方向を決める
	_v = { 0,0,0 };

	// カメラの向いている角度を取得
	float sx = _cam->GetPos().x - _cam->GetTarget().x;
	float sz = _cam->GetPos().z - _cam->GetTarget().z;
	float camrad = atan2(sz, sx);
	float rad = 0.0f;

	//左スティック値
	lStickX = fLx;
	lStickZ = fLz;

	// ローカル入力ベクトル（キーボード）
	vec3::Vec3 inputLocal = vec3::VGet(0.0f, 0.0f, 0.0f);

	// 操作（キーボード）
	if (key & PAD_INPUT_UP)
	{
		inputLocal.x = -1.0f;
	}
	if (key & PAD_INPUT_DOWN)
	{
		inputLocal.x = 1.0f;
	}
	if (key & PAD_INPUT_LEFT)
	{
		inputLocal.z = -1.0f;
	}
	if (key & PAD_INPUT_RIGHT)
	{
		inputLocal.z = 1.0f;
	}

	// アナログ入力の長さ/角度
	float length = sqrt(lStickX * lStickX + lStickZ * lStickZ);
	float radStick = atan2(lStickX, lStickZ);

	// アナログ左スティック用
	if (length < _fAnalogDeadZone)
	{
		// 入力が小さかったら動かなかったことにする
		length = 0.f;
	}
	else
	{
		length = _fMvSpeed;
	}

	// World-space の移動ベクトル（アニメ/向き用）
	_v.x = cos(radStick + camrad) * length;
	_v.z = sin(radStick + camrad) * length;

	SetTargetRotationFromDirection(_v); // 向きの目標値を移動方向に合わせる

	// 重要：ここで直接 _vPos に加算しない。
	// 移動（位置更新）と床判定は ModeGame::EscapeCollision に委ねる。
	// 代わりに入力ベクトルを保存して、EscapeCollision が使えるようにする。

	// 優先順：アナログ > キーボード
	if (length > 0.0f)
	{
		_vInput = vec3::VGet(lStickZ, 0.0f, lStickX);
		if (vec3::VSize(_vInput) > 0.0f) _vInput = vec3::VNorm(_vInput);
	}
	else
	{
		_vInput = inputLocal;
		if (vec3::VSize(_vInput) > 0.0f) _vInput = vec3::VNorm(_vInput);
	}

	// 十字キー保持での軸ロック開始判定
	if (key & PAD_INPUT_6)
	{
		_iAxisHoldCount++;
		if (_iAxisHoldCount >= _iAxisThreshold)
		{
			// ロックに入る瞬間に一度だけロック方向をキャプチャする
			if (!_bAxisUseLock)
			{
				_bAxisUseLock = true;

				// 現在の向きをロック方向として保存
				_vAxisLockDir = _vDir;
				_vAxisLockDir.y = 0.0f;
				if (vec3::VSize(_vAxisLockDir) > 0.0f)
				{
					_vAxisLockDir = vec3::VNorm(_vAxisLockDir);
				}
				else
				{
					// 向きが不定の場合はデフォルト方向を採用
					_vAxisLockDir = vec3::VGet(0.0f, 0.0f, -1.0f);
				}
			}
		}
	}
	else
	{
		_iAxisHoldCount = 0;
		_bAxisUseLock = false;
	}

	// 十字キー水平入力ロック処理
	if (_bAxisUseLock)
	{
		// 軸ロック中の移動処理（向き固定で前後左右に移動）
		vec::Vec3 axis_lock_input = vec3::VGet(0.0f, 0.0f, 0.0f);

		// 軸ロック専用の入力を取得
		if (key & PAD_INPUT_DOWN)
		{
			axis_lock_input.x = 0.5f;
		}
		if (key & PAD_INPUT_UP)
		{
			axis_lock_input.x = -0.5f;
		}
		if (key & PAD_INPUT_LEFT)
		{
			axis_lock_input.z = -0.5f;
		}
		if (key & PAD_INPUT_RIGHT)
		{
			axis_lock_input.z = 0.5f;
		}

		_vInput = axis_lock_input; // 上書き（軸ロック優先）

		// 入力があれば軸ロック移動を計算（アニメ用 _v は更新）
		if (vec3::VSize(axis_lock_input) > 0.0f)
		{
			vec::Vec3 forward = _vAxisLockDir;
			forward.y = 0.0f;
			if (vec3::VSize(forward) > 0.0f)
			{
				forward = vec3::VNorm(forward);
			}
			// 右ベクトル（XZ平面で前方の90度回転）
			//VECTOR right = VGet(forward.z, 0.0f, -forward.x);

			// ローカル入力 axis_lock_input.x = 前後(UP/DOWN), axis_lock_input.z = 左右(LEFT/RIGHT)
			float forwardInput = -axis_lock_input.x; // UP = 前進, DOWN = 後退
			float sideInput = axis_lock_input.z;     // RIGHT = 右移動, LEFT = 左移動

			// 移動ベクトルを計算（前方向 + 横方向）
			vec::Vec3 moveDir = vec3::VGet(0.0f, 0.0f, 0.0f);
			// right ベクトルを直接計算して組み合わせる
			vec::Vec3 right = vec3::VGet(forward.z, 0.0f, -forward.x);

			moveDir.x = forward.x * forwardInput + right.x * sideInput;
			moveDir.z = forward.z * forwardInput + right.z * sideInput;

			// 移動ベクトルを正規化してから速度を掛ける（アニメ向け）
			if (vec3::VSize(moveDir) > 0.0f)
			{
				moveDir = vec3::VNorm(moveDir);
				_v.x = moveDir.x * _fMvSpeed;
				_v.z = moveDir.z * _fMvSpeed;
			}

			// 向きは固定
			_vDir = forward;
		}
	}
	else
	{
		// 通常：カメラ方向に合わせて回転して移動（inputLocal はローカル入力）
		// vをrad分回転させる（ローカル入力の角度）
		if (vec3::VSize(inputLocal) > 0.0f)
		{
			float lengthLocal = _fMvSpeed;
			float localRad = atan2(inputLocal.z, inputLocal.x);
			_v.x = cos(localRad + camrad) * lengthLocal;
			_v.z = sin(localRad + camrad) * lengthLocal;

			// キーボード入力の場合、_vInput を上書き（アナログ優先だが無ければキーボードを使用）
			if (vec3::VSize(_vInput) == 0.0f)
			{
				_vInput = inputLocal;
				if (vec3::VSize(_vInput) > 0.0f) _vInput = vec3::VNorm(_vInput);
			}
		}

	}
	
	// 地上移動（アニメ・向き更新のみ。実際の座標更新は ModeGame::EscapeCollision に任せる）
	if(vec3::VSize(_v) > 0.0f)
	{
		if(_bAxisUseLock)
		{
			// 向きを固定（移動は v のまま）
			_vDir.x = _vAxisLockDir.x;
			_vDir.z = _vAxisLockDir.z;
		}
		else
		{
			_vDir = _v;
		}

		_status = STATUS::WALK;
	}
	else
	{
		_status = STATUS::WAIT;
	}

	if(_status == STATUS::WALK && (_bAxisUseLock || (key & PAD_INPUT_6)))
	{
		// 既に優先アニメが再生中なら何もしない
			// 既存アニメ停止
			if(_animId != -1)
			{
				AnimationManager::GetInstance()->Stop(_animId);
				_animId = -1;
			}
			// 優先アニメを再生
			_animName = "tanuhuman_walk_yoko";
			_animId = AnimationManager::GetInstance()->Play(_handle, _animName, true);
			float anim_speed = 0.5f;
			_fPlayTime += anim_speed;
			if(_animId != -1)
			{
				AnimationManager::GetInstance()->SetTime(_animId, _fPlayTime);
			}
	}
	else if(_status == STATUS::WALK)
	{
		if(_animId != -1)
		{
			AnimationManager::GetInstance()->Stop(_animId);
			_animId = -1;
		}
		// 優先アニメを再生
		_animName = "walk";
		_animId = AnimationManager::GetInstance()->Play(_handle, _animName, true);
		float anim_speed = 0.5f;
		_fPlayTime += anim_speed;
		if(_animId != -1)
		{
			AnimationManager::GetInstance()->SetTime(_animId, _fPlayTime);
		}
	}

	if(_status == STATUS::WALK && !(key & PAD_INPUT_6))
	{
		UpdateRotation();
	}

	if(old_status != _status)
	{
		PlayerSoundMove();
	}

	if(_animId != -1 && !AnimationManager::GetInstance()->IsPlaying(_animId))
	{
		_animId = -1;
		std::string anim_name;
		switch(_status)
		{
		case STATUS::WAIT:
			anim_name = "idle"; // 元コードに合わせる
			break;
		case STATUS::WALK:
			if(_bAxisUseLock || (key & PAD_INPUT_6))
			{
				anim_name = "tanuhuman_walk_yoko";
			}
			else
			{
				anim_name = "walk";
			}
			break;
		default:
			anim_name.clear();
		}
		if(!anim_name.empty())
		{
			_animId = AnimationManager::GetInstance()->Play(_handle, anim_name, true);
			_fPlayTime = 0.0f;
			if(_animId != -1)
			{
				AnimationManager::GetInstance()->SetTime(_animId, _fPlayTime);
			}
		}
	}

	// アニメーション時間・アタッチ管理
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

		std::string anim_name;
		switch(_status)
		{
		case STATUS::WAIT:
			anim_name = "idle";
			break;
		case STATUS::WALK:
			if(_bAxisUseLock )
			{
				anim_name = "tanuhuman_walk_yoko";
			}
			else
			{
				anim_name = "walk";
			}
			break;

		default:
			anim_name.clear();
		}

		if(!anim_name.empty())
		{
			_animId = AnimationManager::GetInstance()->Play(_handle, anim_name, true);
			_fPlayTime = 0.0f;
			switch(_status)
			{
			case STATUS::WAIT:
				_fPlayTime += rand() % 30;
				break;
			}
			if(_animId != -1)
			{
				AnimationManager::GetInstance()->SetTime(_animId, _fPlayTime);
			}
		}
	}
	if(_fTotalTime > 0.0f && _fPlayTime >= _fTotalTime)
	{
		_fPlayTime = 0.0f;
	}

	if(_animId != -1)
	{
		AnimationManager::GetInstance()->SetTime(_animId, _fPlayTime);
	}

	return true;
}

// 描画処理
bool Player::Render()
{
	base::Render();
	// 再生時間をセットする
	//MV1SetAttachAnimTime(_handle, static_cast<int>(_iAttachIndex), _fPlayTime);

	float vorty = GetRotationY();// モデルが標準でどちらを向いているかで式が変わる(これは-zを向いている場合)

	MATRIX mRotY = MGetRotY(vorty);

	//MATRIX mRotZ = MGetRotZ(DX_PI_F * 0.5f); // -90度（必要に応じて符号を反転）

	MATRIX mTrans = MGetTranslate(DxlibConverter::VecToDxLib(_vPos));

	MATRIX mScale = MGetScale(VGet(1.4f, 1.4f, 1.4f));

	MATRIX m = MGetIdent();

	//m = MMult(m, mRotZ);
	m = MMult(m, mRotY);
	m = MMult(m, mScale);
	m = MMult(m, mTrans);

	MV1SetMatrix(_handle, m);

	// 描画
	MV1DrawModel(_handle);

	return true;

}