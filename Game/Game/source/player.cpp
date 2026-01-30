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
#include "TextUtil.h"

// 初期化
bool Player::Initialize()
{
	if(!base::Initialize()) { return false; }
	_handle = MV1LoadModel("res/Tanuhuman/TanuHuman.mv1");
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
	_fMvSpeed = 6.0f;

	_iAxisHoldCount = 0;		// 十字キー水平入力保持カウント初期化
	_bAxisUseLock = false;			// 十字キー水平入力ロックフラグ初期化
	_iAxisThreshold = 15;		// 十字キー水平入力保持閾値設定

	_vAxisLockDir = vec3::VGet(0.0f, 0.0f, -1.0f);

	// カメラオフセット初期化
	_camOffset = vec3::VGet(0.0f, 0.0f, 0.0f);
	_camTargetOffset = vec3::VGet(0.0f, 0.0f, 0.0f);

	// 円形移動用パラメータ初期化
	_arc_pivot_dist = 50.0f;    // 回転中心までの距離（調整可）
	_arc_turn_speed = 1.0f;     // 円形移動速度係数（調整可）
	
	_bLand = true;
	
	// 初期体力設定
	_fHp = 20.0f;

	// 設定ファイルから上書き読み込み
	CFile cfgFile("res/Player/player_config.txt");
	if(cfgFile.Success())
	{
		auto config = TextUtil::ParseKeyValueConfig(cfgFile.DataStr());
		// 移動速度
		auto it = config.find("speed");
		if(it != config.end())
		{
			float val;
			// 変換成功したら上書き
			if(TextUtil::TryParseFloat(it->second, val))
			{
				_fMvSpeed = val;
			}
		}
		// 体力
		it = config.find("hp");
		if(it != config.end())
		{
			float val;
			if(TextUtil::TryParseFloat(it->second, val))
			{
				_fHp = val;
			}
		}
	}
	return true;
}

// 終了
bool Player::Terminate()
{
	base::Terminate();
	return true;
}

// 計算処理
bool Player::Process()
{
	int key = ApplicationBase::GetInstance()->GetKey();
	int trg = ApplicationBase::GetInstance()->GetTrg();

	base::Process();

	// 処理前の位置を保存
	_vOldPos = _vPos;

	// 処理前のステータスを保存しておく
	CharaBase::STATUS old_status = _status;
	// 移動方向を決める
    _v = { 0,0,0 };
	
	// カメラの向いている角度を取得
	float sx = _cam->_vPos.x - _cam->_vTarget.x;
	float sz = _cam->_vPos.z - _cam->_vTarget.z;
	float camrad = atan2(sz, sx);
	float rad = 0.0f;

	//左スティック値
    lStickX = fLx;
    lStickZ = fLz;

	// ローカル入力ベクトル
	vec3::Vec3 inputLocal = vec3::VGet(0.0f, 0.0f, 0.0f);

	if((key & (PAD_INPUT_7 | PAD_INPUT_8)) == 0)
	{
		// キャラ移動(カメラ設定に合わせて)

	    // 操作
		if (CheckHitKey(KEY_INPUT_UP))
		{
			inputLocal.z = -1.0f;
		}
		if (CheckHitKey(KEY_INPUT_DOWN))
		{
			inputLocal.z = 1.0f;
		}
		if (CheckHitKey(KEY_INPUT_LEFT))
		{
			inputLocal.x = -1.0f;
		}
		if (CheckHitKey(KEY_INPUT_RIGHT))
		{
			inputLocal.x = 1.0f;
		}

		// ローカル入力ベクトルを計算
		float length = sqrt(lStickX * lStickX + lStickZ * lStickZ);
		float rad = atan2(lStickX, lStickZ);

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

		// vをrad分回転させる
		if (vec3::VSize(_v) > 0.f) { length = _fMvSpeed; }
		_v.x = cos(rad + camrad) * length;
		_v.z = sin(rad + camrad) * length;

		// 処理前の位置を保存
		_vOldPos = _vPos;

		// vの分移動
		_vPos = vec3::VAdd(_vPos, _v);

        // 十字キー保持での軸ロック開始判定
		if(key & PAD_INPUT_6)
		{
			_iAxisHoldCount++;
			if(_iAxisHoldCount >= _iAxisThreshold)
			{
				// ロックに入る瞬間に一度だけロック方向をキャプチャする
				if(!_bAxisUseLock)
				{
					_bAxisUseLock = true;
					
					// 現在の向きをロック方向として保存
					_vAxisLockDir = _vDir;
					_vAxisLockDir.y = 0.0f;
					if(vec3::VSize(_vAxisLockDir) > 0.0f)
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
		if(_bAxisUseLock)
		{
			// 軸ロック中の移動処理（向き固定で前後左右に移動）
			vec::Vec3 axis_lock_input = vec3::VGet(0.0f, 0.0f, 0.0f);

			// 軸ロック専用の入力を取得
			if(key & PAD_INPUT_DOWN) 
			{
				axis_lock_input.x = 0.5f;
			}
			if(key & PAD_INPUT_UP) 
			{
				axis_lock_input.x = -0.5f;
			}
			if(key & PAD_INPUT_LEFT) 
			{
				axis_lock_input.z = -0.5f;
			}
			if(key & PAD_INPUT_RIGHT) 
			{
				axis_lock_input.z = 0.5f;
			}

			_vInput = axis_lock_input;

			// 入力があれば軸ロック移動を計算
			if(vec3::VSize(axis_lock_input) > 0.0f)
			{
				vec::Vec3 forward = _vAxisLockDir;
				forward.y = 0.0f;
				if(vec3::VSize(forward) > 0.0f)
				{
					forward = vec3::VNorm(forward);
				}
				// 右ベクトル（XZ平面で前方の90度回転）
				VECTOR right = VGet(forward.z, 0.0f, -forward.x);

				// ローカル入力 axis_lock_input.x = 前後(UP/DOWN), axis_lock_input.z = 左右(LEFT/RIGHT)
				float forwardInput = -axis_lock_input.x; // UP = 前進, DOWN = 後退
				float sideInput = axis_lock_input.z;     // RIGHT = 右移動, LEFT = 左移動

				// 移動ベクトルを計算（前方向 + 横方向）
				vec::Vec3 moveDir = vec3::VGet(0.0f, 0.0f, 0.0f);
				moveDir.x = forward.x * forwardInput + right.x * sideInput;
				moveDir.z = forward.z * forwardInput + right.z * sideInput;

				// 移動ベクトルを正規化してから速度を掛ける
				if(vec3::VSize(moveDir) > 0.0f)
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
            if(vec3::VSize(inputLocal) > 0.0f)
            {
                length = _fMvSpeed;
                float localRad = atan2(inputLocal.z, inputLocal.x);
                _v.x = cos(localRad + camrad) * length;
                _v.z = sin(localRad + camrad) * length;
            }

        }
	}

	// 地上移動
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

	if(_animId != -1 && !AnimationManager::GetInstance()->IsPlaying(_animId))
	{
		_animId = -1;
		std::string anim_name;
		switch(_status)
		{
		case STATUS::WAIT:
			anim_name = "mot_attack_charge_loop"; // 元コードに合わせる
			break;
		case STATUS::WALK:
			anim_name = "mot_move_run";
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
            anim_name = "mot_attack_charge_loop";
            break;
        case STATUS::WALK:
            anim_name = "mot_move_run";
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
	if(_fPlayTime >= _fTotalTime)
	{
		_fPlayTime = 0.0f;
	}

	return true;
}

// 描画処理
bool Player::Render()
{
	base::Render();
	// 再生時間をセットする
	//MV1SetAttachAnimTime(_handle, static_cast<int>(_iAttachIndex), _fPlayTime);
	
	float vorty = atan2(_vDir.x * -1, _vDir.z * -1);// モデルが標準でどちらを向いているかで式が変わる(これは-zを向いている場合)

	MATRIX mRotY = MGetRotY(vorty);

	//MATRIX mRotZ = MGetRotZ(DX_PI_F * 0.5f); // -90度（必要に応じて符号を反転）

	MATRIX mTrans = MGetTranslate(DxlibConverter::VecToDxLib(_vPos));

	MATRIX mScale = MGetScale(VGet(1.0f, 1.0f, 1.0f));

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