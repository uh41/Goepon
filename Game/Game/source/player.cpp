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
	_mv_speed = 6.0f;
	// ジャンプ、重力関連初期化
	_land = true;
	_jump_height = 20;
	_gravity = 0.0f;
	// ダッシュ関連初期化
	_is_dashing = false;
	_dash_speed = 30.0f;
	_dash_time = 12.0f;
	_dash_timer = 0.0f;
	_dash_direction = VGet(0.0f, 0.0f, 0.0f);

	// ドッジロール関連初期化
	_is_rolling = false;						// ドッジロール中かどうか
	_roll_speed = 20.0f;						// ドッジロール速度
	_roll_time = 14.0f;						// ドッジロール継続時間
	_roll_timer = 0.0f;						// ドッジロール残り時間
	_roll_direction = VGet(0.0f, 0.0f, 0.0f);	// ドッジロール方向

	_jump_count = true;
	_air_control = 1.0f;

	_air_attack_used = false;	// 空中攻撃フラグ初期化

	_axis_hold_count = 0;		// 十字キー水平入力保持カウント初期化
	_axis_lock = false;			// 十字キー水平入力ロックフラグ初期化
	_is_attack = false;			// 攻撃中フラグ初期化
	_hold_threshold = 15;		// 十字キー水平入力保持閾値設定

	_axis_lock_dir = VGet(0.0f, 0.0f, -1.0f);

	// 円形移動用パラメータ初期化
	_arc_pivot_dist = 50.0f;	// 回転中心までの距離（調整可）
	_arc_turn_speed = 1.0f;		// 円形移動速度係数（調整可）
	return true;
}

// 終了
bool Player::Terminate()
{
	base::Terminate();
	return true;
}

// 攻撃アクション
bool Player::Attack()
{
	AttackCapsule
	(
		VGet(0.0f, 0.0f, 0.0f),		// カプセルの下位置(剣の根元)
		VGet(0.0f, 0.0f, 150.0f),	// カプセルの上位置(剣の先端)
		50.0f,						// 半径
		15,							// 発生までの時間
		10,							// 有効時間
		0,							// カプセルの伸縮速度
		true,						// カプセルがキャラに追従するか
		10.0f,						// ダメージ量
		10,							// ノックバックフレーム数
		_dir						// 攻撃方向
	);

	return true;
}

// 攻撃用カプセル当たり判定登録
bool Player::AttackCapsule(VECTOR underpos, VECTOR overpos, float r, int waittime, int activetime, int timespeed, bool follow, float damage, int framenum, VECTOR dir)
{
	// 攻撃用カプセル当たり判定情報を追加
	mymath::ATTACKCOLLISION attackcollision;
	attackcollision.capsule.underpos = underpos;
	attackcollision.capsule.overpos = overpos;
	attackcollision.capsule.r = r;
	if(follow)
	{
		attackcollision.capsule.modelhandle = _handle;
		attackcollision.capsule.framenum = MV1SearchFrame(_handle, "Character1_RightHandMiddle1"); // 右手中指先端に追従
	}
	attackcollision.waittime = waittime;		// 発生までの時間
	attackcollision.activetime = activetime;	// 有効時間
	attackcollision.damage = damage;			// ダメージ量
	attackcollision.follow = follow;			// 追従するかどうか
	attackcollision.attackChara = "Player";		// 攻撃をしたキャラクターの名前
	attackcollision.isHit = false;				// 当たっていない状態で追加

	_attack_collision.push_back(attackcollision);
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

	if(_land)
	{
		_gravity = 0.0f;
	}

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
        _input_v = inputLocal;

        // 十字キー保持での軸ロック開始判定
		if(key & PAD_INPUT_4)
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

			_input_v = axis_lock_input;
			
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
					v.x = moveDir.x * _mv_speed;
					v.z = moveDir.z * _mv_speed;
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
                length = _mv_speed;
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

	// 以下は既存のロジック（攻撃、ジャンプ、空中処理等）。v はここからワールド移動量として使われます。

	if(trg & PAD_INPUT_1)
	{
		if(_land == true)
		{
			// ジャンプ
			_jump_count = false;
			_gravity = _jump_height;
			_pos.y += _gravity;
			_status = STATUS::JUMP;
			_land = false;
		}
		else
		{
			// 空中かつダッシュしていない場合、ダッシュ開始
			if(!_is_dashing)
			{
				_is_dashing = true;
				_dash_timer = _dash_time;
				_status = STATUS::DASHING;
				// ダッシュ方向は現在の入力方向
				if(VSize(v) > 0.0f)
				{
					_dash_direction = v;
					_dash_direction.y = 0.0f;

					if(VSize(_dash_direction) > 0.0f)
					{
						_dash_direction = VNorm(_dash_direction);
					}
				}
				else
				{
					_dash_direction = _dir;
					_dash_direction.y = 0.0f;

					if(VSize(_dash_direction) > 0.0f)
					{
						_dash_direction = VNorm(_dash_direction);
					}
					else
					{
						_dash_direction = VGet(0.0f, 0.0f, -1.0f);
					}
				}
			}

		}
	}

	// ドッジロール
	if(_land == true && (trg & PAD_INPUT_3) && !_is_rolling && !_is_dashing)
	{
		_is_rolling = true;
		_roll_timer = _roll_time;
		_status = STATUS::ROLLING;
		// ドッジロール方向は現在の入力方向
		VECTOR dir = { 0,0,0 };
		if(VSize(v) > 0.0f)
		{
			dir = v;
		}
		else
		{
			dir = _dir;
			dir.y = 0.0f;
			if(VSize(dir) <= 0.0f)
			{
				dir = VGet(0.0f, 0.0f, -1.0f);
			}
		}

		// 正規化
		if(VSize(dir) > 0.0f)
		{
			dir = VNorm(dir);
		}

		_roll_direction = dir;
	}

	// 攻撃入力（PAD_INPUT_2）
	if(trg & PAD_INPUT_2)
	{
		// 地上にいる場合、または空中で未使用の場合のみ攻撃可能
		if(_status != STATUS::ATTACK)
		{
			// 地上にいる場合は常に攻撃可能
			if(_land)
			{
				_status = STATUS::ATTACK;
				Attack();
			}
			// 空中にいて、まだ空中攻撃を使っていない場合のみ攻撃可能
			else if(!_air_attack_used)
			{
				_status = STATUS::ATTACK;
				Attack();
				_air_attack_used = true; // 空中攻撃を使用した
			}
		}
	}

	// 攻撃中は移動・ジャンプ処理をしない
	if(_status == STATUS::ATTACK)
	{
		// 攻撃時の前進（省略せず既存処理）
		if(_play_time < _total_time * 0.5f)
		{
			float attack_move_speed;
			if(_land) attack_move_speed = 5.0f;
			else      attack_move_speed = 8.0f;
			VECTOR forward_dir = VNorm(_dir);
			_pos.x += forward_dir.x * attack_move_speed;
			_pos.z += forward_dir.z * attack_move_speed;
		}

		// 空中で攻撃している場合は重力を適用
		if(!_land)
		{
			_gravity -= 0.5f;
			_pos.y += _gravity;

			if(_pos.y <= 0.0f)
			{
				_pos.y = 0.0f;
				_land = true;
				_gravity = 0.0f;
				_jump_count = true;
				_air_attack_used = false;
			}
		}
	}
	else if(_land == false)
	{
		// 重力処理
		_gravity -= 0.98f;
		_pos.y += _gravity;
		if(_gravity < 0.0f) _status = STATUS::FALL;
		if(_pos.y <= 0.0f)
		{
			_pos.y = 0.0f;
			_land = true;
			_gravity = 0.0f;
			_jump_count = true;
			_is_dashing = false;
			_dash_timer = 0.0f;
			_air_attack_used = false;
		}

		// ダッシュ処理
		if(_is_dashing)
		{
			if(_dash_timer > 0.0f)
			{
				_pos.x += _dash_direction.x * _dash_speed;
				_pos.z += _dash_direction.z * _dash_speed;
				_dash_timer -= 1.0f;
			}
			else
			{
				_is_dashing = false;
				_status = STATUS::FALL;
			}
		}
		else
		{
			// 空中通常移動（v はワールド移動量）
			if(VSize(v) > 0.0f)
			{
				_pos.x += v.x * _air_control;
				_pos.z += v.z * _air_control;
				// 軸ロック中は向きを固定
				if(_axis_lock)
				{
					_dir.x = _axis_lock_dir.x;
					_dir.z = _axis_lock_dir.z;
				}
				else
				{
					_dir = v;
				}
			}
		}
	}
	else
	{
		if(_is_rolling)
		{
			if(_roll_timer > 0.0f)
			{
				float ratio = _roll_timer / _roll_time;
				float speed = _roll_speed * (ratio * ratio);
				_pos.x += _roll_direction.x * speed;
				_pos.z += _roll_direction.z * speed;
				_dir = _roll_direction;
				_roll_timer -= 1.0f;
				_status = STATUS::ROLLING;
			}
			else
			{
				_is_rolling = false;
				_status = STATUS::WAIT;
			}
		}
		else
		{
			// 地上移動（v はワールド移動量）
			if(VSize(v) > 0.0f && _status != STATUS::ATTACK)
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
				if(old_status == STATUS::FALL || old_status == STATUS::LANDING)
				{
					_status = STATUS::LANDING;
				}
			}
		}

	}

	// アニメーション時間・アタッチ管理（既存処理そのまま）
	if(old_status == _status)
	{
		float anim_speed = 0.5f;
		if(_status == STATUS::ATTACK) anim_speed = 2.0f;
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
		case STATUS::JUMP:
			_attach_index = MV1AttachAnim(_handle, MV1GetAnimIndex(_handle, "mot_move_jump_f_start"), -1, FALSE);
			break;
		case STATUS::FALL:
			_attach_index = MV1AttachAnim(_handle, MV1GetAnimIndex(_handle, "mot_move_jump_f_downloop"), -1, FALSE);
			break;
		case STATUS::LANDING:
			_attach_index = MV1AttachAnim(_handle, MV1GetAnimIndex(_handle, "mot_move_land"), -1, FALSE);
			break;
		case STATUS::DASHING:
			_attach_index = MV1AttachAnim(_handle, MV1GetAnimIndex(_handle, "mot_attack_charge_step"), -1, FALSE);
			break;
		case STATUS::ATTACK:
			_attach_index = MV1AttachAnim(_handle, MV1GetAnimIndex(_handle, "mot_attack_nomal"), -1, FALSE);
			break;
		case STATUS::ROLLING:
			_attach_index = MV1AttachAnim(_handle, MV1GetAnimIndex(_handle, "mot_move_jump_f_uploop"), -1, FALSE);
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
		if(_status == STATUS::LANDING)
		{
			_status = STATUS::WAIT;
			if(_attach_index != -1)
			{
				MV1DetachAnim(_handle, _attach_index);
				_attach_index = -1;
			}
			_attach_index = MV1AttachAnim(_handle, MV1GetAnimIndex(_handle, "mot_attack_charge_loop"), -1, FALSE);
			_total_time = MV1GetAttachAnimTotalTime(_handle, _attach_index);
		}
		else if(_status == STATUS::ATTACK)
		{
			if(_land)
			{
				_status = STATUS::WAIT;
				if(_attach_index != -1)
				{
					MV1DetachAnim(_handle, _attach_index);
					_attach_index = -1;
				}
				_attach_index = MV1AttachAnim(_handle, MV1GetAnimIndex(_handle, "mot_attack_charge_loop"), -1, FALSE);
				_total_time = MV1GetAttachAnimTotalTime(_handle, _attach_index);
			}
			else
			{
				_status = STATUS::FALL;
				if(_attach_index != -1)
				{
					MV1DetachAnim(_handle, _attach_index);
					_attach_index = -1;
				}
				_attach_index = MV1AttachAnim(_handle, MV1GetAnimIndex(_handle, "mot_move_jump_f_downloop"), -1, FALSE);
				_total_time = MV1GetAttachAnimTotalTime(_handle, _attach_index);
			}
		}
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