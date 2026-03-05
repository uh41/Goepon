#include "TreasureBase.h"

bool TreasureBase::Initialize()
{
	base::Initialize();

	_hitCollisionFrame = -1;
	_openCollisionFrame = -1;
	_objStatus = OBJSTATUS::NONE;

	_isOpen = false;
	_isVisible = true;
	_openedNotified = false;

	_vPos = vec::Vec3(0.0f, 0.0f, 0.0f);
	_vDir = vec::Vec3{ 0.0f, 0.0f, -1.0f };

	return true;
}

bool TreasureBase::Terminate()
{
	_hitCollisionFrame = -1;
	_openCollisionFrame = -1;

	_isVisible = false;
	_isOpen = false;
	_openedNotified = false;

	_objStatus = OBJSTATUS::NONE;

	// ObjectBase::Terminate() は MV1DeleteModel するので呼ぶ
	return base::Terminate();

	return true;
}

bool TreasureBase::Process()
{
	base::Process();
	return true;
}

bool TreasureBase::Render()
{
	base::Render();
	return true;
}

MATRIX TreasureBase::MakeModelMatrix() const
{
	const float rotXRad = 12.0f;
	float vorty = atan2(_vDir.x * -1, _vDir.z * -1);
	const float tiltX = DX_PI_F * (rotXRad) / 180.0f; // 10度前傾

	MATRIX mRotY = MGetRotY(vorty);
	MATRIX mRotX = MGetRotX(tiltX);
	MATRIX mTrans = MGetTranslate(DxlibConverter::VecToDxLib(_vPos));
	MATRIX mScale = MGetScale(VGet(1.2f, 1.2f, 1.2f));
	MATRIX m = MGetIdent();
	m = MMult(m, mScale);
	m = MMult(m, mRotY);
	m = MMult(m, mRotX);
	m = MMult(m, mTrans);
	return m;
}

// デバック：開いたら見た目を消す
void TreasureBase::SetOpen(bool isOpen)
{
	_isOpen = isOpen;

	// デバッグ：開いたら見た目を消す
	if(_isOpen)
	{
		_isVisible = false;
	}
}