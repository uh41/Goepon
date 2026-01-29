#include "Treasure.h"


namespace
{

	inline void ApplyMatrixAndRefreshCollInfo(int handle, int hitFrame, int openFrame, const MATRIX& m)
	{

		if(handle < 0)
		{
			return;
		}


		MV1SetMatrix(handle, m);


		if(hitFrame >= 0)
		{
			MV1RefreshCollInfo(handle, hitFrame);
		}
		if(openFrame >= 0)
		{
			MV1RefreshCollInfo(handle, openFrame);
		}
	}
}

bool Treasure::Initialize()
{
	base::Initialize();


	_handle = MV1LoadModel("res/Treasure/tuzura_02.mv1");
	if(_handle < 0) { DxLib::printfDx("Treasure model load failed\n"); return false; }


	_hitCollisionFrame = MV1SearchFrame(_handle, "Collision_04");
	_openCollisionFrame = MV1SearchFrame(_handle, "Collision_05");

	if(_hitCollisionFrame < 0) { DxLib::printfDx("HitFrame not found\n"); return false; }

	// if (_openCollisionFrame < 0) { DxLib::printfDx("OpenFrame not found\n"); }


	_attachIndex = -1;

	_objStatus = OBJSTATUS::NONE;

	_vPos = vec::Vec3{ 600.0f, 0.0f, 2450.0f };
	_vDir = vec::Vec3{ 0.0f, 0.0f, -1.0f };


	MV1SetupCollInfo(_handle, _hitCollisionFrame, 16, 16, 16);
	MV1SetFrameVisible(_handle, _hitCollisionFrame, FALSE);

	if(_openCollisionFrame >= 0)
	{
		MV1SetupCollInfo(_handle, _openCollisionFrame, 16, 16, 16);
		MV1SetFrameVisible(_handle, _openCollisionFrame, FALSE);
	}


	_isOpen    = false;
	_isVisible = true;

	ApplyMatrixAndRefreshCollInfo(_handle, _hitCollisionFrame, _openCollisionFrame, MakeModelMatrix());
	return true;
}


bool Treasure::Terminate()
{
	MV1DeleteModel(_handle);
	return true;
}

bool Treasure::Process()
{
	base::Process();

	ApplyMatrixAndRefreshCollInfo(_handle, _hitCollisionFrame, _openCollisionFrame, MakeModelMatrix());


	if(!_isOpen && _objStatus != OBJSTATUS::OPEN)
	{
		if(_attachIndex != -1)
		{
			MV1DetachAnim(_handle, _attachIndex);
			_attachIndex = -1;
		}
		_objStatus = OBJSTATUS::OPEN;
	}
	return true;
}

bool Treasure::Render()
{
	base::Render();

	// 非表示なら描画しない（デバック)
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


MATRIX Treasure::MakeModelMatrix() const
{
	float vorty = atan2(_vDir.x * -1, _vDir.z * -1);
	MATRIX mRotY = MGetRotY(vorty);
	MATRIX mTrans = MGetTranslate(DxlibConverter::VecToDxLib(_vPos));
	MATRIX mScale = MGetScale(VGet(1.2f, 1.2f, 1.2f));
	MATRIX m = MGetIdent();
	m = MMult(m, mScale);
	m = MMult(m, mRotY);
	m = MMult(m, mTrans);
	return m;
}