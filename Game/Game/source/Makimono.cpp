#include "Makimono.h"

bool Makimono::Initialize()
{
	base::Initialize();
	
	if(!LoadModel(mv1::makimono_01))
	{
		// ì«Ç›çûÇ›é∏îs
		return false;
	}

	_vPos = vec::Vec3{ 0.0f, 0.0f,  0.0f };
	_vDir = vec::Vec3{ 0.0f, 0.0f, -1.0f };
	haveMakimono = false;
	_isVisible = true;
	_drawSize = 60.0f;

	ModelMatrixSetUp();

	return true;
}

bool Makimono::Terminate()
{
	base::Terminate();
	haveMakimono = false;
	_isVisible = false;
	return true;
}

bool Makimono::Process()
{
	base::Process();

	ModelMatrixSetUp();
	return true;
}

bool Makimono::Render()
{
	base::Render();

	if(!_isVisible) { return true; }

	float vorty = atan2(_vDir.x * -1, _vDir.z * -1);

	MATRIX mRotY = MGetRotY(vorty);

	MV1SetPosition(_handle, DxlibConverter::VecToDxLib(_vPos));

	MATRIX mTrans = MGetTranslate(DxlibConverter::VecToDxLib(_vPos));
	MATRIX mScale = MGetScale(VGet(1.8f, 1.8f, 1.8f));

	MATRIX m = MGetIdent();
	m = MMult(m, mRotY);
	m = MMult(m, mScale);
	m = MMult(m, mTrans);

	MV1SetMatrix(_handle, m);
	MV1DrawModel(_handle);

	return true;
}
