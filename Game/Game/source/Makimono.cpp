#include "Makimono.h"

bool Makimono::Initialize()
{
	base::Initialize();
	
	_handle = LoadGraph("res/Makimono/Makimono.png");
	if(_handle < 0) { return false; }

	_vPos = vec::Vec3{ 0.0f, 0.0f, 0.0f };
	_vDir = vec::Vec3{ 0.0f, 0.0f, -1.0f };
	haveMakimono = false;
	_isVisible = true;
	_drawSize = 60.0f;
	return true;
}

bool Makimono::Terminate()
{
	if(_handle >= 0)
	{
		DeleteGraph(_handle);
		_handle = -1;
	}
	haveMakimono = false;
	_isVisible = false;
	return true;
}

bool Makimono::Process()
{
	base::Process();
	return true;
}

bool Makimono::Render()
{
	base::Render();
	if(!_isVisible || _handle < 0)
	{
		return true;
	}
	SetUseZBuffer3D(TRUE);
	SetWriteZBuffer3D(TRUE);

	int w = 0, h = 0;
	GetGraphSize(_handle, &w, &h);

	const float cx = w * 0.5f;
	const float cy = h * 0.5f;

	DrawBillboard3D(
		VGet(_vPos.x, _vPos.y, _vPos.z),
		cx,
		cy,
		_drawSize,
		0.0f,
		_handle,
		TRUE
	);
	
	return true;
}
