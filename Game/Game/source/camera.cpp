/*********************************************************************/
// * \file   camera.cpp
// * \brief  カメラクラス
// *
// * \author 鈴木裕稀
// * \date   2025/12/15
// * \作業内容: 新規作成 鈴木裕稀　2025/12/15
/*********************************************************************/

#include "camera.h"
#include "appframe.h"

bool Camera::Initialize()
{
	// カメラの設定(わかりやすい位置に)
	_vPos = VGet(0.0f, 600.0f, -300.0f);
	_vTarget = VGet(0.0f, 60.0f, 0.0f);
	_fClipNear = 2.0f;
	_fClipFar = 10000.0f;
	_fForvScale = -10.0f;

	return true;
}

bool Camera::Process()
{
	int key = ApplicationMain::GetInstance()->GetKey();
	int trg = ApplicationMain::GetInstance()->GetTrg();
	// カメラ操作
	if((key & (PAD_INPUT_7 | PAD_INPUT_8)) == (PAD_INPUT_7 | PAD_INPUT_8))
	{
		// 距離、ターゲットの高さ変更
		float sx = _vPos.x - _vTarget.x;
		float sz = _vPos.z - _vTarget.z;
		float rad = atan2(sz, sx);
		float length = sqrt(sz * sz + sx * sx);
		if(key & PAD_INPUT_LEFT)
		{
			length -= 1.0f;
		}
		if (key & PAD_INPUT_RIGHT)
		{
			length += 1.0f;
		}
		_vPos.x = _vTarget.x + cos(rad) * length;
		_vPos.z = _vTarget.z + sin(rad) * length;

		// Y位置
		if(key & PAD_INPUT_DOWN)
		{
			_vTarget.y -= 1.0f;
		}
		if(key & PAD_INPUT_UP)
		{
			_vTarget.y += 1.0f;
		}
	}
	else if(key & PAD_INPUT_7)
	{
		// 角度変更
		// Y軸回転
		float sx = _vPos.x - _vTarget.x;
		float sz = _vPos.z - _vTarget.z;
		float rad = atan2(sz, sx);
		float length = sqrt(sz * sz + sx * sx);
		if(key & PAD_INPUT_LEFT)
		{
			rad -= 0.05f;
		}
		if(key & PAD_INPUT_RIGHT)
		{
			rad += 0.05f;
		}
		_vPos.x = _vTarget.x + cos(rad) * length;
		_vPos.z = _vTarget.z + sin(rad) * length;

		// Y軸位置
		if(key & PAD_INPUT_DOWN)
		{
			_vPos.y -= 5.0f;
		}
		if(key & PAD_INPUT_UP)
		{
			_vPos.y += 5.0f;
		}
	}
	else if(key & PAD_INPUT_8)
	{
		// カメラ位置(注目位置もXZスライド)
		float sx = _vPos.x - _vTarget.x;
		float sz = _vPos.z - _vTarget.z;
		float camrad = atan2(sz, sx);

		// 移動方向を決める
		VECTOR v = { 0,0,0 };
		float mvSpeed = 2.0f;
		if(key & PAD_INPUT_DOWN)
		{
			v.x = 1;
		}
		if(key & PAD_INPUT_UP)
		{
			v.x = -1;
		}
		if(key & PAD_INPUT_LEFT)
		{
			v.z = -1;
		}
		if(key & PAD_INPUT_RIGHT)
		{
			v.z = 1;
		}

		// vをrad分回転させる
		float length = 0.0f;
		if(VSize(v) > 0.0f)
		{
			length = mvSpeed;
		}
		float rad = atan2(v.z, v.x);
		v.x = cos(rad + camrad) * length;
		v.z = sin(rad + camrad) * length;

		// vの分移動
		_vPos = VAdd(_vPos, v);
		_vTarget = VAdd(_vTarget, v);
	}
	return true;
}

bool Camera::Render()
{
	int x = 0, y = 0, size = 16;
	SetFontSize(size);
	DrawFormatString(x, y, GetColor(255, 0, 0), "Camera:"); y += size;
	DrawFormatString(x, y, GetColor(255, 0, 0), "  target = (%5.2f, %5.2f, %5.2f)", _vTarget.x, _vTarget.y, _vTarget.z); y += size;
	DrawFormatString(x, y, GetColor(255, 0, 0), "  pos    = (%5.2f, %5.2f, %5.2f)", _vPos.x, _vPos.y, _vPos.z); y += size;
	float sx = _vPos.x - _vTarget.x;
	float sz = _vPos.z - _vTarget.z;
	float length = sqrt(sz * sz + sx * sx);
	float rad = atan2(sz, sx);
	float deg = RAD2DEG(rad);
	DrawFormatString(x, y, GetColor(255, 0, 0), "  len = %5.2f, rad = %5.2f, deg = %5.2f", length, rad, deg); y += size;
	return true;
}

void Camera::MoveBy(const VECTOR& delta)
{
	_vPos = VAdd(_vPos, delta);
	_vTarget = VAdd(_vTarget, delta);
}

void Camera::ZoomTowardsTarget(float amount)
{
	// 方向ベクトル = target - pos
	VECTOR dir = VSub(_vTarget, _vPos);
	// 長さ
	float len = sqrtf(dir.x * dir.x + dir.y * dir.y + dir.z * dir.z);
	if(len <= 0.0f) 
	{
		return;
	} // 近すぎたら無視
	// 正規化 -> dir * (amount / len)
	VECTOR step = VScale(dir, amount / len);
	_vPos = VAdd(_vPos, step);

	// オプション: 最短距離や最長距離でクランプしたければここで制限を入れる
}

// ターゲットを中心にY軸回転する（deltaRadは回転ラジアン）
void Camera::RotateAroundTarget(float deltaRad)
{
    // 現在の水平距離と角度を取得
    float sx = _vPos.x - _vTarget.x;
    float sz = _vPos.z - _vTarget.z;
    float length = sqrtf(sx * sx + sz * sz);
	if(length <= 0.0f) 
	{
		return;
	}
    float rad = atan2f(sz, sx);
    rad += deltaRad;
    _vPos.x = _vTarget.x + cosf(rad) * length;
    _vPos.z = _vTarget.z + sinf(rad) * length;
}