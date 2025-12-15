#include "player.h"
#include "appframe.h"

// 初期化
bool Player::Initialize()
{
	if(!base::Initialize()) { return false; }
	_handle = MV1LoadModel("res/Player/Player.mv1");
	_attach_index = -1;
	// ステータスを「無し」に設定
	_status = STATUS::NONE;
	// 再生時間の初期化
	_total_time = 0.0f;
	_play_time = 0.0f;
	// 位置、向きの初期化
	_pos = VGet(0.0f, 0.0f, 0.0f); // 初期位置が同じだが、押し出され処理のおかげで位置がずれる
	_dir = VGet(0.0f, 0.0f, -1.0f);// キャラモデルはデフォルトで-Z方向を向いている
	// 腰位置の設定
	_col_sub_y = 40.0f;
	// コリジョン半径の設定
	_collision_r = 30.0f;
	_collision_weight = 20.0f;
	_cam = nullptr;
	_mvSpeed = 6.0f;

	_axis_hold_count = 0;		// 十字キー水平入力保持カウント初期化
	_axis_lock = false;			// 十字キー水平入力ロックフラグ初期化
	_hold_threshold = 15;		// 十字キー水平入力保持閾値設定

	_axis_lock_dir = VGet(0.0f, 0.0f, -1.0f);

	// 円形移動用パラメータ初期化
	_arc_pivot_dist = 50.0f;	// 回転中心までの距離（調整可）
	_arc_turn_speed = 1.0f;		// 円形移動速度係数（調整可）
	
	_land = true;
	
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
	_old_pos = _pos;

	// 補間ターン速度（ラジアン／フレーム）: 小さくするとゆっくり回る
	const float AXIS_TURN_SPEED = 0.12f; // 調整可

	// 処理前のステータスを保存しておく
	CharaBase::STATUS old_status = _status;
	VECTOR v = { 0,0,0 };
	float length = 0.0f;

	// カメラの向いている角度を取得
	float sx = _cam->_v_pos.x - _cam->_v_target.x;
	float sz = _cam->_v_pos.z - _cam->_v_target.z;
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
        _inputV = inputLocal;

        // 十字キー保持での軸ロック開始判定
		if(key & PAD_INPUT_6)
		{
			_axis_hold_count++;
			if(_axis_hold_count >= _hold_threshold)
			{
				// ロックに入る瞬間に一度だけロック方向をキャプチャする
				if(!_axis_lock)
				{
					_axis_lock = true;
					
					// 現在の向きをロック方向として保存
					_axis_lock_dir = _dir;
					_axis_lock_dir.y = 0.0f;
					if(VSize(_axis_lock_dir) > 0.0f)
					{
						_axis_lock_dir = VNorm(_axis_lock_dir);
					}
					else
					{
						// 向きが不定の場合はデフォルト方向を採用
						_axis_lock_dir = VGet(0.0f, 0.0f, -1.0f);
					}
				}
			}
		}
		else
		{
			_axis_hold_count = 0;
			_axis_lock = false;
		}

		// 十字キー水平入力ロック処理
		if(_axis_lock)
		{
			// 軸ロック中の移動処理（向き固定で前後左右に移動）
			VECTOR axis_lock_input = VGet(0.0f, 0.0f, 0.0f);
			
			// 軸ロック専用の入力を取得
			if(key & PAD_INPUT_DOWN) { axis_lock_input.x = 0.5f; }
			if(key & PAD_INPUT_UP) { axis_lock_input.x = -0.5f; }
			if(key & PAD_INPUT_LEFT) { axis_lock_input.z = -0.5f; }
			if(key & PAD_INPUT_RIGHT) { axis_lock_input.z = 0.5f; }

			_inputV = axis_lock_input;
			
			// 入力があれば軸ロック移動を計算
			if(VSize(axis_lock_input) > 0.0f)
			{
				VECTOR forward = _axis_lock_dir;
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
					v.x = moveDir.x * _mvSpeed;
					v.z = moveDir.z * _mvSpeed;
				}
				else
				{
					v.x = 0.0f;
					v.z = 0.0f;
				}

				// 向きは固定
				_dir = forward;
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
                length = _mvSpeed;
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
		if(_axis_lock)
		{
			// 向きを固定（移動は v のまま）
			_dir.x = _axis_lock_dir.x;
			_dir.z = _axis_lock_dir.z;
		}
		else
		{
			_dir = v;
		}

		_status = STATUS::WALK;
	}
	else
	{
		_status = STATUS::WAIT;
	}

	// 地上位置への移動
	_pos.x += v.x;
	_pos.z += v.z;

	// アニメーション時間・アタッチ管理
	if(old_status == _status)
	{
		float anim_speed = 0.5f;
		_play_time += anim_speed;
		switch(_status)
		{
		case STATUS::WAIT:
			_play_time += (float)(rand() % 10) / 100.0f;
			break;
		}
	}
	else
	{
		if(_attach_index != -1)
		{
			MV1DetachAnim(_handle, _attach_index);
			_attach_index = -1;
		}
		switch(_status)
		{
		case STATUS::WAIT:
			_attach_index = MV1AttachAnim(_handle, MV1GetAnimIndex(_handle, "mot_attack_charge_loop"), -1, FALSE);
			break;
		case STATUS::WALK:
			_attach_index = MV1AttachAnim(_handle, MV1GetAnimIndex(_handle, "mot_move_run"), -1, FALSE);
			break;
		}
		_total_time = MV1GetAttachAnimTotalTime(_handle, _attach_index);
		_play_time = 0.0f;
		switch(_status)
		{
		case STATUS::WAIT:
			_play_time += rand() % 30;
			break;
		}
	}
	if(_play_time >= _total_time)
	{
		_play_time = 0.0f;
	}

	return true;
}

// 描画処理
bool Player::Render()
{
	base::Render();
	// 再生時間をセットする
	MV1SetAttachAnimTime(_handle, _attach_index, _play_time);

	// 位置
	MV1SetPosition(_handle, _pos);
	// 向きからY軸回転を算出
	VECTOR vrot = { 0,0,0, };
	vrot.y = atan2(_dir.x * -1, _dir.z * -1);// モデルが標準でどちらを向いているかで式が変わる(これは-zを向いている場合)
	MV1SetRotationXYZ(_handle, vrot);
	// 描画
	MV1DrawModel(_handle);

	return true;

}