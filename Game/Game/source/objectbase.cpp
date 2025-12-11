#include "objectbase.h"

// 初期化
bool ObjectBase::Initialize()
{
	_cam = nullptr;
	_handle = -1;
	_half_polygon_size = 0.0f;
	_diffuse = GetColorU8(255, 255, 255, 255);
	_specular = GetColorU8(0, 0, 0, 0);
	_u_list = { 0.0f, 0.0f, 1.0f, 1.0f };
	_v_list = { 0.0f, 1.0f, 0.0f, 1.0f };

	return true;
}

// 終了
bool ObjectBase::Terminate()
{
	return true;
}
// 計算処理
bool ObjectBase::Process()
{
	return true;
}
// 描画処理
bool ObjectBase::Render()
{
	return true;
}
