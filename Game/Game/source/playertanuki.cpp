/*********************************************************************/
// * \file   playertanuki.cpp
// * \brief  狸状態クラス
// *
/*********************************************************************/

#include "playertanuki.h"
#include "appframe.h"

// 初期化
bool PlayerTanuki::Initialize()
{
	if(!base::Initialize()) { return false; }

	_handle = MV1LoadModel("res/Tanuki/goepon.mv1");
	_iAttachIndex = -1;
	// ステータスを「無し」に設定
	_status = STATUS::NONE;
	// 再生時間の初期化
	_fTotalTime = 0.0f;
	_fPlayTime = 0.0f;
	// 位置、向きの初期化
	//if(vec3::VSize(_vPos) == 0.0f)
	//{
	//	_vPos = vec3::VGet(0.0f, 0.0f, 0.0f); // 初期位置が同じだが、押し出され処理のおかげで位置がずれる
	//}
	//_vDir = vec3::VGet(0.0f, 0.0f, -1.0f);// キャラモデルはデフォルトで-Z方向を向いている
	// 腰位置の設定
	_fColSubY = 40.0f;
	// コリジョン半径の設定
	_fCollisionR = 30.0f;
	_fCollisionWeight = 20.0f;
	_cam = nullptr;
	_fMvSpeed = 10.0f;

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

	// 前フレーム情報の保存
	_vOldPos = _vPos;
	// 古いステータスの保存
	CharaBase::STATUS old_status = _status;

	// 入力 → 移動ベクトルの計算
	_v = { 0,0,0 };

	// カメラの向いている角度を取得
	float sx = _cam->_vPos.x - _cam->_vTarget.x;
	float sz = _cam->_vPos.z - _cam->_vTarget.z;
	float camrad = atan2(sz, sx);

	// 左スティック
	lStickX = fLx;
	lStickZ = fLz;

	// キーボード入力
	vec::Vec3 inputLocal = vec3::VGet(0.0f, 0.0f, 0.0f);
	if(CheckHitKey(KEY_INPUT_UP))
	{
		inputLocal.z = 1.0f;
	}
	if(CheckHitKey(KEY_INPUT_DOWN))
	{
		inputLocal.z = -1.0f;
	}
	if(CheckHitKey(KEY_INPUT_LEFT))
	{
		inputLocal.x = -1.0f;
	}
	if(CheckHitKey(KEY_INPUT_RIGHT))
	{
		inputLocal.x = 1.0f;
	}

	
	// スティックの傾きから移動量を計算

	// 後段で参照しても未定義にならないように、ここで宣言しておく
		// スティックの傾きから移動量を計算
	float length = sqrtf(lStickX * lStickX + lStickZ * lStickZ);

	// 入力角（ローカル）
	float localRad = 0.0f;

	// デッドゾーンを超えたら移動、未満なら停止
	if(length >= _fAnalogDeadZone)
	{
		// 入力座標系（ローカル）
		// 右=+X、上=+Z に合わせる（Yは使用しない）
		const float moveX = lStickZ;
		const float moveZ = lStickX;

		// 入力方向(向き)を保存（CharaBase の GetInputVector 用）
		_vInput = vec3::VGet(moveX, 0.0f, moveZ);

		// 入力方向の角度（ローカル空間）
		localRad = atan2f(moveZ, moveX);

		// 一定速度で移動（倒し具合で速度を変えたいなら length を使う）
		const float speed = _fMvSpeed;

		// カメラ角で回転したワールド移動量
		_v.x = cosf(localRad + camrad) * length;
		_v.z = sinf(localRad + camrad) * length;

		_vDir = _v;
		_status = STATUS::WALK;
	}
	else
	{
		// デッドゾーン：動かない
		_v = { 0.0f, 0.0f, 0.0f };
		_vInput = vec3::VGet(0.0f, 0.0f, 0.0f);
		_status = STATUS::WAIT;
	}


	if(_fPlayTime >= _fTotalTime)
	{
		_fPlayTime = 0.0f;
	}

	// アニメーション名取得用ラムダ
	auto GetAnimName = [this](STATUS name) -> std::string
		{
			switch(name)
			{
			case STATUS::WAIT:
				return "taiki";
			case STATUS::WALK:
				return "walk";
			default:
				return std::string();
			}
		};

	// アニメーション再生用ラムダ
	auto PlayAnim = [&](bool change)
		{
			std::string name = GetAnimName(_status);
			if(name.empty()) { return; }

			_animId = AnimationManager::GetInstance()->Play(_handle, name, true);
			_fPlayTime = 0.0f;

			// ステータス変更時はランダムで再生時間をずらす
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

	// --- アニメーション管理 ---
	// 再生中のアニメーションが終了しているかチェック（非ループ再生は AnimationManager がインスタンスを削除する）
	if(_animId != -1 && !AnimationManager::GetInstance()->IsPlaying(_animId))
	{
		_animId = -1;
		PlayAnim(false);
	}

	//既存のアニメ管理（ステータス変化時の処理）
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

		PlayAnim(true);// ステータス変更時なのでtrue
	}

	if(_fPlayTime >= _fTotalTime)
	{
		_fPlayTime = 0.0f;
	}


	// --- ここで実際に位置とカメラを移動させる ---
	//if(vec3::VSize(_v) > 0.0f)
	//{
	//	//// プレイヤーの位置を移動
	//	_vPos = vec3::VAdd(_vPos, _v);

	//	//// カメラが設定されていればカメラ位置はプレイヤー位置 + オフセットで設定（加算はしない）
	//	//if(_cam != nullptr)
	//	//{
	//	//	_cam->_vPos = vec3::VAdd(_vPos, _camOffset);
	//	//	_cam->_vTarget = vec3::VAdd(_vPos, _camTargetOffset);
	//	//}
	//}
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

	DrawFormatString(10, 90, GetColor(255, 255, 0), "fLx=%.3f fLz=%.3f", fLx, fLz);

	return true;
}