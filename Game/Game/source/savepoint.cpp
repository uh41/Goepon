#include "savepoint.h"

SavePoint::SavePoint()
{
	_savePointCollisionFrame = -1;
	_attachIndex = -1;
	_isVisible = false;
	Initialize();
}

bool SavePoint::Initialize()
{
	base::Initialize();
	LoadModel(mv1::S_Savepoint);
	_savePointCollisionFrame = FindCollisonFrameAnyCase(_handle);
	_attachIndex = -1;
	_vPos = vec::Vec3{ 0.0f, 0.0f, 0.0f };
	_vDir = vec::Vec3{ 0.0f, 0.0f, -1.0f };
	MV1SetupCollInfo(_handle, _savePointCollisionFrame, 16, 16, 16);
	MV1SetFrameVisible(_handle, _savePointCollisionFrame, FALSE);
	_isVisible = true;
	ApplyMatrixAndRefreshCollInfo(_handle, _savePointCollisionFrame, -1, MakeModelMatrix());
	return true;
}



bool SavePoint::Terminate()
{
	return true;
}

int SavePoint::FindCollisonFrameAnyCase(int handle)
{
	if(handle == -1)
	{
		return -1;
	}

	int frame = MV1SearchFrame(handle, "Collision");
	if(frame != -1)
	{
		return frame;
	}

	frame = MV1SearchFrame(handle, "collision");
	if(frame != -1)
	{
		return frame;
	}

	int frameNum = MV1GetFrameNum(handle);
	for(int fi = 0; fi < frameNum; ++fi)
	{
		const char* fname = MV1GetFrameName(handle, fi);
		if(!fname)
		{
			continue;
		}
		std::string s(fname);
		if(s.find("Collision") != std::string::npos || s.find("collision") != std::string::npos)
		{
			return fi;
		}
	}

	return -1;
}

MATRIX SavePoint::MakeModelMatrix() const
{
	float vorty = atan2(_vDir.x * -1, _vDir.z * -1);
	MATRIX mRotY = MGetRotY(vorty);
	MATRIX mTrans = MGetTranslate(DxlibConverter::VecToDxLib(_vPos));
	MATRIX mScale = MGetScale(VGet(1.0f, 1.0f, 1.0f));
	MATRIX m = MGetIdent();
	m = MMult(m, mScale);
	m = MMult(m, mRotY);
	m = MMult(m, mTrans);
	return m;
}

bool SavePoint::Process()
{
	base::Process();

	ApplyMatrixAndRefreshCollInfo(_handle, _savePointCollisionFrame, -1, MakeModelMatrix());
	return true;
}

bool SavePoint::Render()
{
	base::Render();

	if(!_isVisible)
	{
		return true;
	}

	if(_handle != -1)
	{
		MV1DrawModel(_handle);
	}

	return true;
}