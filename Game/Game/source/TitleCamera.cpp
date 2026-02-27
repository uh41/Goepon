#include "TitleCamera.h"

bool TitleCamera::Initialize()
{
	// ƒJƒƒ‰‚Ìİ’è
	_vPos = vec3::VGet(0.0f, 100.0f, 0.0f);
	_vTarget = vec3::VGet(0.0f, 0.0f, -1.0f);
	_clipNear = 2.0f;
	_clipFar = 10000.0f;
	_forvScale = -10.0f;

	return true;
}

bool TitleCamera::Process()
{
	return true;
}

bool TitleCamera::Render()
{
	base::Render();
	return true;
}
