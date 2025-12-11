#include "camera.h"
#include "appframe.h"

bool Camera::Initialize()
{
	// カメラの設定(わかりやすい位置に)
	_v_pos = VGet(0.0f, 600.0f, -300.0f);
	_v_target = VGet(0.0f, 60.0f, 0.0f);
	_clip_near = 2.0f;
	_clip_far = 10000.0f;

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
		float sx = _v_pos.x - _v_target.x;
		float sz = _v_pos.z - _v_target.z;
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
		_v_pos.x = _v_target.x + cos(rad) * length;
		_v_pos.z = _v_target.z + sin(rad) * length;

		// Y位置
		if(key & PAD_INPUT_DOWN)
		{
			_v_target.y -= 1.0f;
		}
		if(key & PAD_INPUT_UP)
		{
			_v_target.y += 1.0f;
		}
	}
	else if(key & PAD_INPUT_7)
	{
		// 角度変更
		// Y軸回転
		float sx = _v_pos.x - _v_target.x;
		float sz = _v_pos.z - _v_target.z;
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
		_v_pos.x = _v_target.x + cos(rad) * length;
		_v_pos.z = _v_target.z + sin(rad) * length;

		// Y軸位置
		if(key & PAD_INPUT_DOWN)
		{
			_v_pos.y -= 5.0f;
		}
		if(key & PAD_INPUT_UP)
		{
			_v_pos.y += 5.0f;
		}
	}
	else if(key & PAD_INPUT_8)
	{
		// カメラ位置(注目位置もXZスライド)
		float sx = _v_pos.x - _v_target.x;
		float sz = _v_pos.z - _v_target.z;
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
		_v_pos = VAdd(_v_pos, v);
		_v_target = VAdd(_v_target, v);
	}
	return true;
}

bool Camera::Render()
{
	int x = 0, y = 0, size = 16;
	SetFontSize(size);
	DrawFormatString(x, y, GetColor(255, 0, 0), "Camera:"); y += size;
	DrawFormatString(x, y, GetColor(255, 0, 0), "  target = (%5.2f, %5.2f, %5.2f)", _v_target.x, _v_target.y, _v_target.z); y += size;
	DrawFormatString(x, y, GetColor(255, 0, 0), "  pos    = (%5.2f, %5.2f, %5.2f)", _v_pos.x, _v_pos.y, _v_pos.z); y += size;
	float sx = _v_pos.x - _v_target.x;
	float sz = _v_pos.z - _v_target.z;
	float length = sqrt(sz * sz + sx * sx);
	float rad = atan2(sz, sx);
	float deg = RAD2DEG(rad);
	DrawFormatString(x, y, GetColor(255, 0, 0), "  len = %5.2f, rad = %5.2f, deg = %5.2f", length, rad, deg); y += size;
	return true;
}
