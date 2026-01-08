/*********************************************************************/
// * \file   playertanuki.cpp
// * \brief  狸状態クラス
// *
// * \author 鈴木裕稀
// * \date   2025/12/15
// * \作業内容: 新規作成 鈴木裕稀　2025/12/15
/*********************************************************************/

#include "playertanuki.h"
#include "appframe.h"

// 初期化
bool PlayerTanuki::Initialize()
{
	if(!base::Initialize()) { return false; }
	
	_iHandle = MV1LoadModel("res/SDChar/SDChar.mv1");
	_iAttachIndex = -1;
	// ステータスを「無し」に設定
	_status = STATUS::NONE;
	// 再生時間の初期化
	_fTotalTime = 0.0f;
	_fPlayTime = 0.0f;
	// 位置、向きの初期化
	_vPos = VGet(0.0f, 0.0f, 0.0f);
	_vDir = VGet(0.0f, 0.0f, -1.0f);// キャラモデルはデフォルトで-Z方向を向いている
	// 腰位置の設定
	_fColSubY = 40.0f;
	// コリジョン半径の設定
	_fCollisionR = 30.0f;
	_fCollisionWeight = 20.0f;
	_cam = nullptr;
	_fMvSpeed = 6.0f;

	_bLand = true;

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
	_vOldPos = _vPos;

	// 処理前のステータスを保存しておく
	CharaBase::STATUS old_status = _status;
	VECTOR v = { 0,0,0 };
	float length = 0.0f;

	// カメラの向いている角度を取得
	float sx = _cam->_vPos.x - _cam->_vTarget.x;
	float sz = _cam->_vPos.z - _cam->_vTarget.z;
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
	_vInput = inputLocal;

	// カメラ方向に合わせて移動量を計算
	if(VSize(inputLocal) > 0.0f)
	{
		length = _fMvSpeed;
		float localRad = atan2(inputLocal.z, inputLocal.x);
		v.x = cos(localRad + camrad) * length;
		v.z = sin(localRad + camrad) * length;
		_vDir = v;
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
			_iAttachIndex = MV1AttachAnim(_iHandle, MV1GetAnimIndex(_iHandle, "idle"), -1, FALSE);
			break;
		case STATUS::WALK:
			_iAttachIndex = MV1AttachAnim(_iHandle, MV1GetAnimIndex(_iHandle, "run"), -1, FALSE);
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

	// --- ここで実際に位置とカメラを移動させる ---
	if(VSize(v) > 0.0f)
	{
		// プレイヤーの位置を移動
		_vPos = VAdd(_vPos, v);

		// カメラが設定されていればカメラ位置はプレイヤー位置 + オフセットで設定（加算はしない）
		if(_cam != nullptr)
		{
			_cam->_vPos = VAdd(_vPos, _camOffset);
			_cam->_vTarget = VAdd(_vPos, _camTargetOffset);
		}
	}

	return true;
}

// 描画処理
bool PlayerTanuki::Render()
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