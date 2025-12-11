#include "playertanuki.h"
#include "appframe.h"

// 初期化
bool PlayerTanuki::Initialize()
{
	if(!base::Initialize()) { return false; }
	
	_handle = MV1LoadModel("res/SDChar/SDChar.mv1");
	_attach_index = -1;
	// ステータスを「無し」に設定
	_status = STATUS::NONE;
	// 再生時間の初期化
	_total_time = 0.0f;
	_play_time = 0.0f;
	// 位置、向きの初期化
	_pos = VGet(0.0f, 0.0f, 0.0f);
	_dir = VGet(0.0f, 0.0f, -1.0f);// キャラモデルはデフォルトで-Z方向を向いている
	// 腰位置の設定
	_col_sub_y = 40.0f;
	// コリジョン半径の設定
	_collision_r = 30.0f;
	_collision_weight = 20.0f;
	_cam = nullptr;
	_mv_speed = 6.0f;

	_land = true;

	return true;
}

// 終了
bool PlayerTanuki::Terminate()
{
	base::Terminate();

	return true;
}

// 計算処理
bool PlayerTanuki::Process()
{
	base::Process();

	int key = ApplicationBase::GetInstance()->GetKey();

	// 処理前の位置を保存
	_old_pos = _pos;

	// 補間ターン速度（ラジアン／フレーム）
	const float AXIS_TURN_SPEED = 0.12f;

	// 処理前のステータスを保存しておく
	CharaBase::STATUS old_status = _status;
	VECTOR v = { 0,0,0 };
	float length = 0.0f;

	// カメラの向いている角度を取得
	float sx = _cam->_v_pos.x - _cam->_v_target.x;
	float sz = _cam->_v_pos.z - _cam->_v_target.z;
	float camrad = atan2(sz, sx);

	// キャラ移動(カメラ設定に合わせて)
	VECTOR inputLocal = VGet(0.0f, 0.0f, 0.0f);
	if(key & PAD_INPUT_DOWN) {
		inputLocal.x = 1;
	}
	if(key & PAD_INPUT_UP) {
		inputLocal.x = -1;
	}
	if(key & PAD_INPUT_LEFT) {
		inputLocal.z = -1;
	}
	if(key & PAD_INPUT_RIGHT) {
		inputLocal.z = 1;
	}

	// 入力ベクトルを保存（EscapeCollisionで使用）
	_input_v = inputLocal;

	// カメラ方向に合わせて移動量を計算
	if(VSize(inputLocal) > 0.0f)
	{
		length = _mv_speed;
		float localRad = atan2(inputLocal.z, inputLocal.x);
		v.x = cos(localRad + camrad) * length;
		v.z = sin(localRad + camrad) * length;
		_dir = v;
		_status = STATUS::WALK;
	}
	else
	{
		v = VGet(0.0f, 0.0f, 0.0f);
		_status = STATUS::WAIT;
	}

	// アニメーション管理
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
			_attach_index = MV1AttachAnim(_handle, MV1GetAnimIndex(_handle, "idle"), -1, FALSE);
			break;
		case STATUS::WALK:
			_attach_index = MV1AttachAnim(_handle, MV1GetAnimIndex(_handle, "run"), -1, FALSE);
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
bool PlayerTanuki::Render()
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