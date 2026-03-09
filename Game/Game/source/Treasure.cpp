/*********************************************************************/
// * \file   treasure.cpp
// * \brief  お宝クラス
// *
// * \author 石森虹大
// * \date   2026/1/25
// * \作業内容: 新規作成 石森虹大 2026/1/25
//			 
/*********************************************************************/
#include "Treasure.h"

bool Treasure::Initialize()
{
	base::Initialize();


	LoadModel("res/Treasure/tuzura_05.mv1");
	if(_handle < 0) { DxLib::printfDx("Treasure model load failed\n"); return false; }


	_hitCollisionFrame = MV1SearchFrame(_handle, "Collision_04");
	_openCollisionFrame = MV1SearchFrame(_handle, "Collision_05");

	if(_hitCollisionFrame < 0) { DxLib::printfDx("HitFrame not found\n"); return false; }

	// if (_openCollisionFrame < 0) { DxLib::printfDx("OpenFrame not found\n"); }


	_attachIndex = -1;

	_objStatus = OBJSTATUS::NONE;

	_vPos = vec::Vec3{ 714.0f, 0.0f, 5803.0f };
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
	//_handle = -1;
	//_hitCollisionFrame = -1;
	//_openCollisionFrame = -1;
	//_isVisible = false;
	//_isOpen = false;
	//_attachIndex = -1;

	return true;
}

bool Treasure::Process()
{
	base::Process();

	ApplyMatrixAndRefreshCollInfo(_handle, _hitCollisionFrame, _openCollisionFrame, MakeModelMatrix());

	if(!_isOpen && _objStatus != OBJSTATUS::OPEN)
	{
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

