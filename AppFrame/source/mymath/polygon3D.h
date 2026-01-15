#pragma once
#include "vector3.h"
#include "mysegment.h"

class Polygon3D
{
public:

	Polygon3D(const vec::Vec3& v1, const vec::Vec3& v2, const vec::Vec3& v3)
	{
		_vVertex1 = v1;
		_vVertex2 = v2;
		_vVertex3 = v3;
	}

	// 法線ベクトル
	vec::Vec3 NormalVector() const
	{
		//return vec::Vec3::Cross((_vVertex2 - _vVertex1), (_vVertex3 - _vVertex1)).Normalized();
	}

	vec::Vec3 _vVertex1;
	vec::Vec3 _vVertex2;
	vec::Vec3 _vVertex3;
};
