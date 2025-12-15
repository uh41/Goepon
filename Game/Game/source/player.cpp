/*********************************************************************/
// * \file   player.cpp
// * \brief  人狸状態クラス
// *
// * \author 鈴木裕稀
// * \date   2025/12/15
// * \作業内容: 新規作成 鈴木裕稀　2025/12/15
/*********************************************************************/

#include "player.h"
#include "appframe.h"

// 初期化
bool Player::Initialize()
{
	if(!base::Initialize()) { return false; }
	_iHandle = MV1LoadModel("res/Player/Player.mv1");
	_iAttachIndex = -1;
	// ステータスを「無し」に設定
	_status = STATUS::NONE;
	// 再生時間の初期化
	_fTotalTime = 0.0f;
	_fPlayTime = 0.0f;
	// 位置、向きの初期化
	_vPos = VGet(0.0f, 0.0f, 0.0f); // 初期位置が同じだが、押し出され処理のおかげで位置がずれる
	_vDir = VGet(0.0f, 0.0f, -1.0f);// キャラモデルはデフォルトで-Z方向を向いている
	// 腰位置の設定
	_fColSubY = 40.0f;
	// コリジョン半径の設定
	_fCollisionR = 30.0f;
	_fCollisionWeight = 20.0f;
	_cam = nullptr;
	_fMvSpeed = 6.0f;

	_iAxisHoldCount = 0;		// 十字キー水平入力保持カウント初期化
	_bAxisUseLock = false;			// 十字キー水平入力ロックフラグ初期化
	_iAxisThreshold = 15;		// 十字キー水平入力保持閾値設定

	_vAxisLockDir = VGet(0.0f, 0.0f, -1.0f);

	// 円形移動用パラメータ初期化
	_arc_pivot_dist = 50.0f;	// 回転中心までの距離（調整可）
	_arc_turn_speed = 1.0f;		// 円形移動速度係数（調整可）
	
	_bLand = true;
	
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
	base::Process();
	int key = ApplicationBase::GetInstance()->GetKey();
	int trg = ApplicationBase::GetInstance()->GetTrg();

	// 処理前の位置を保存
	_vOldPos = _vPos;

	// 処理前のステータスを保存しておく
	CharaBase::STATUS old_status = _status;
	VECTOR v = { 0,0,0 };
	float length = 0.0f;

	// カメラの向いている角度を取得
	float sx = _cam->_vPos.x - _cam->_vTarget.x;
	float sz = _cam->_vPos.z - _cam->_vTarget.z;
	float camrad = atan2(sz, sx);
	float rad = 0.0f;

	if((key & (PAD_INPUT_7 | PAD_INPUT_8)) == 0)
	{
		// キャラ移動(カメラ設定に合わせて)

        // ローカル入力を取得
        VECTOR inputLocal = VGet(0.0f, 0.0f, 0.0f);
        if(key & PAD_INPUT_DOWN) { inputLocal.x = 1; }
        if(key & PAD_INPUT_UP)   { inputLocal.x = -1; }
        if(key & PAD_INPUT_LEFT) { inputLocal.z = -1; }
        if(key & PAD_INPUT_RIGHT){ inputLocal.z = 1; }

        // 回転前のローカル入力を保存（斜め判定に使う）
        _vInput = inputLocal;

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
					if(VSize(_vAxisLockDir) > 0.0f)
					{
						_vAxisLockDir = VNorm(_vAxisLockDir);
					}
					else
					{
						// 向きが不定の場合はデフォルト方向を採用
						_vAxisLockDir = VGet(0.0f, 0.0f, -1.0f);
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
			VECTOR axis_lock_input = VGet(0.0f, 0.0f, 0.0f);
			
			// 軸ロック専用の入力を取得
			if(key & PAD_INPUT_DOWN) { axis_lock_input.x = 0.5f; }
			if(key & PAD_INPUT_UP) { axis_lock_input.x = -0.5f; }
			if(key & PAD_INPUT_LEFT) { axis_lock_input.z = -0.5f; }
			if(key & PAD_INPUT_RIGHT) { axis_lock_input.z = 0.5f; }

			_vInput = axis_lock_input;
			
			// 入力があれば軸ロック移動を計算
			if(VSize(axis_lock_input) > 0.0f)
			{
				VECTOR forward = _vAxisLockDir;
				forward.y = 0.0f;
				if(VSize(forward) > 0.0f) forward = VNorm(forward);

				// 右ベクトル（XZ平面で前方の90度回転）
				VECTOR right = VGet(forward.z, 0.0f, -forward.x);

				// ローカル入力 axis_lock_input.x = 前後(UP/DOWN), axis_lock_input.z = 左右(LEFT/RIGHT)
				float forwardInput = -axis_lock_input.x; // UP = 前進, DOWN = 後退
				float sideInput = axis_lock_input.z;     // RIGHT = 右移動, LEFT = 左移動

				// 移動ベクトルを計算（前方向 + 横方向）
				VECTOR moveDir = VGet(0.0f, 0.0f, 0.0f);
				moveDir.x = forward.x * forwardInput + right.x * sideInput;
				moveDir.z = forward.z * forwardInput + right.z * sideInput;

				// 移動ベクトルを正規化してから速度を掛ける
				if(VSize(moveDir) > 0.0f)
				{
					moveDir = VNorm(moveDir);
					v.x = moveDir.x * _fMvSpeed;
					v.z = moveDir.z * _fMvSpeed;
				}
				else
				{
					v.x = 0.0f;
					v.z = 0.0f;
				}

				// 向きは固定
				_vDir = forward;
			}
			else
			{
				// 入力がない場合は移動しない
				v = VGet(0.0f, 0.0f, 0.0f);
			}
		}
        else
        {
            // 通常：カメラ方向に合わせて回転して移動（inputLocal はローカル入力）
            // vをrad分回転させる（ローカル入力の角度）
            if(VSize(inputLocal) > 0.0f)
            {
                length = _fMvSpeed;
                float localRad = atan2(inputLocal.z, inputLocal.x);
                v.x = cos(localRad + camrad) * length;
                v.z = sin(localRad + camrad) * length;
            }
            else
            {
                v = VGet(0.0f, 0.0f, 0.0f);
            }
        }
	}

	// 地上移動
	if(VSize(v) > 0.0f)
	{
		if(_bAxisUseLock)
		{
			// 向きを固定（移動は v のまま）
			_vDir.x = _vAxisLockDir.x;
			_vDir.z = _vAxisLockDir.z;
		}
		else
		{
			_vDir = v;
		}

		_status = STATUS::WALK;
	}
	else
	{
		_status = STATUS::WAIT;
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
		if(_iAttachIndex != -1)
		{
			MV1DetachAnim(_iHandle, _iAttachIndex);
			_iAttachIndex = -1;
		}
		switch(_status)
		{
		case STATUS::WAIT:
			_iAttachIndex = MV1AttachAnim(_iHandle, MV1GetAnimIndex(_iHandle, "mot_attack_charge_loop"), -1, FALSE);
			break;
		case STATUS::WALK:
			_iAttachIndex = MV1AttachAnim(_iHandle, MV1GetAnimIndex(_iHandle, "mot_move_run"), -1, FALSE);
			break;
		}
		_fTotalTime = MV1GetAttachAnimTotalTime(_iHandle, _iAttachIndex);
		_fPlayTime = 0.0f;
		switch(_status)
		{
		case STATUS::WAIT:
			_fPlayTime += rand() % 30;
			break;
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
	MV1SetAttachAnimTime(_iHandle, _iAttachIndex, _fPlayTime);

	// 位置
	MV1SetPosition(_iHandle, _vPos);
	// 向きからY軸回転を算出
	VECTOR vrot = { 0,0,0, };
	vrot.y = atan2(_vDir.x * -1, _vDir.z * -1);// モデルが標準でどちらを向いているかで式が変わる(これは-zを向いている場合)
	MV1SetRotationXYZ(_iHandle, vrot);
	// 描画
	MV1DrawModel(_iHandle);

	return true;

}