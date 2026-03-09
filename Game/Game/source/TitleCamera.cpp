#include "TitleCamera.h"

bool TitleCamera::Initialize()
{
	base::Initialize();

	// タイトルカメラの初期設定
	_vPos = vec::Vec3(50.0f, 150.0f, -250.0f);
	_vTarget = vec::Vec3(0.0f, 50.0f, 0.0f);
	_clipNear = 1.0f;
	_clipFar = 10000.0f;
	_forvScale = 1.0f;

	
	return true;
}

bool TitleCamera::Process()
{
	base::Process();
	return true;
}

bool TitleCamera::Render()
{
	base::Render();
	return true;
}
