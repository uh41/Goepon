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
	
	_handle = MV1LoadModel("res/Tanuki/anime_goepon_walk.mv1");
	_iAttachIndex = -1;
	// ステータスを「無し」に設定
	_status = STATUS::NONE;
	// 再生時間の初期化
	_fTotalTime = 0.0f;
	_fPlayTime = 0.0f;
	// 位置、向きの初期化
	_vPos = vec3::VGet(0.0f, 0.0f, 0.0f);
	_vDir = vec3::VGet(0.0f, 0.0f, -1.0f);// キャラモデルはデフォルトで-Z方向を向いている
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
	_v = { 0,0,0 };

	// カメラの向いている角度を取得
	float sx = _cam->_vPos.x - _cam->_vTarget.x;
	float sz = _cam->_vPos.z - _cam->_vTarget.z;
	float camrad = atan2(sz, sx);

	// キャラ移動(カメラ設定に合わせて)
	lStickX = fLx;
	lStickZ = fLz;

	vec::Vec3 inputLocal = vec3::VGet(0.0f, 0.0f, 0.0f);
	if (CheckHitKey(KEY_INPUT_UP))
	{
		lStickZ = -1.0f;
	}
	if (CheckHitKey(KEY_INPUT_DOWN))
	{
		lStickZ = 1.0f;
	}
	if (CheckHitKey(KEY_INPUT_LEFT))
	{
		lStickX = -1.0f;
	}
	if (CheckHitKey(KEY_INPUT_RIGHT))
	{
		lStickX = 1.0f;
	}
	float length = sqrt(lStickX * lStickX + lStickZ * lStickZ);
	float rad = atan2(lStickX, lStickZ);
	if (length < _fAnalogDeadZone)
	{
		length = 0.0f;
	}

	// 入力ベクトルを保存（EscapeCollisionで使用）
	_vInput = inputLocal;

	// カメラ方向に合わせて移動量を計算
	if (length > 0.0f)
	{
		length = _fMvSpeed;
		_v.x = cosf(rad + camrad) * length;
		_v.z = sinf(rad + camrad) * length;

		_vDir = _v;
		_status = STATUS::WALK;
	}
	else
	{
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
            anim_name = "goepon_walk";
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

	// --- ここで実際に位置とカメラを移動させる ---
	if(vec3::VSize(_v) > 0.0f)
	{
		// プレイヤーの位置を移動
		_vPos = vec3::VAdd(_vPos, _v);

		// カメラが設定されていればカメラ位置はプレイヤー位置 + オフセットで設定（加算はしない）
		if(_cam != nullptr)
		{
			_cam->_vPos = vec3::VAdd(_vPos, _camOffset);
			_cam->_vTarget = vec3::VAdd(_vPos, _camTargetOffset);
		}
	}
	return true;
}

// 描画処理
bool PlayerTanuki::Render()
{
	base::Render();

	// 再生時間をセットする
		// 再生時間をセットする
	//MV1SetAttachAnimTime(_handle, static_cast<int>(_iAttachIndex), static_cast<float>(_fPlayTime));

	float vorty = atan2(_vDir.x * -1, _vDir.z * -1);// モデルが標準でどちらを向いているかで式が変わる(これは-zを向いている場合)

	MATRIX mRotY = MGetRotY(vorty);

	MATRIX mRotZ = MGetRotZ(DX_PI_F * 0.5f); // -90度（必要に応じて符号を反転）

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