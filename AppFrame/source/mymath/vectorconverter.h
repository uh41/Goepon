#pragma once
#include "vector3operator.h"
#include "../container.h"

class VectorConverter
{
public:
	// DxLib‚©‚çVector3‚Ö•ÏŠ·
	static VECTOR VecToDxLib(vec::Vec3 v1)
	{
		return VGet(v1.x, v1.y, v1.z);
	}

	// Vector3‚©‚çDxLib‚Ö•ÏŠ·
	static vec::Vec3 DxLibToVec(VECTOR v1)
	{
		return vec3::VGet(v1.x, v1.y, v1.z);
	}

	
};
