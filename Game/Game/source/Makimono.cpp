#include "Makimono.h"

bool Makimono::Initialize()
{
	base::Initialize();
	//LoadModel("res/Makimono/makimono.mv1");
	if(_handle < 0) { DxLib::printfDx("Makimono model load failed\n"); return false; }
	_vPos = vec::Vec3{ 0.0f, 0.0f, 0.0f };
	_vDir = vec::Vec3{ 0.0f, 0.0f, -1.0f };
	haveMakimono = true;
	_isVisible = true;
	return true;
}

bool Makimono::Terminate()
{
	_handle = -1;
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
	if (!_isVisible)
	{
		return true; 
	}
	if(_handle >= 0)
	{
		MV1DrawModel(_handle);
	}
	return true;
}
