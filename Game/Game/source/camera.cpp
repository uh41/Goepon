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
	_vPos = vec3::VGet(0.0f, 1600.0f, -662.0f);
	_vTarget = vec3::VGet(0.0f, 60.0f, 0.0f);
	_clipNear = 2.0f;
	_clipFar = 10000.0f;
	_forvScale = -10.0f;

	return true;
}

bool Camera::Terminate()
{
	// Camera 自体は外部リソースを保持していないため、状態を初期化して終了とする
	_vPos = vec3::VGet(0.0f, 0.0f, 0.0f);
	_vTarget = vec3::VGet(0.0f, 0.0f, 0.0f);
	_clipNear = 0.0f;
	_clipFar = 0.0f;
	_forvScale = 0.0f;

	return true;
}

bool Camera::Process()
{
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

void Camera::MoveBy(const vec::Vec3& delta)
{
	_vPos = vec3::VAdd(_vPos, delta);
	_vTarget = vec3::VAdd(_vTarget, delta);
}

void Camera::ZoomTowardsTarget(float amount)
{
	// 方向ベクトル = target - pos
	vec::Vec3 dir = vec3::VSub(_vTarget, _vPos);
	// 長さ
	float len = sqrtf(dir.x * dir.x + dir.y * dir.y + dir.z * dir.z);
	if(len <= 0.0f) 
	{
		return;
	} // 近すぎたら無視
	// 正規化 -> dir * (amount / len)
	vec::Vec3 step = vec3::VScale(dir, amount / len);
	_vPos = vec3::VAdd(_vPos, step);

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