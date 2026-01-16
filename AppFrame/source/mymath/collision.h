#pragma once
#include "polygon3D.h"
#include "mysegment.h"
#include "vector3.h"

class Collision
{
public:

	// 点と線分の最短点を求める
	static const vec::Vec3 SegPointLatestPoint(vec::Vec3 p, Segment s, float* t)
	{
		vec::Vec3 v1 = p - s.start;
		vec::Vec3 v2 = s.end - s.start;

		float v = v1.Dot(v2) / v2.Dot(v2);

		vec::Vec3 result(s.start + v2 * v); // 最短点

		// 端点判定
		if(v1.Dot(v2) < 0.0f)
		{
			result = s.start;
		}
		else if(vec::Vec3::Dot(p - s.end, s.start - s.end) < 0.0f)
		{
			result = s.end;
		}
		if(t)
		{
			*t = v;
		}

		return result;
	}

	// 点と線分の最短距離の2乗を求める
	static const float SetPointDistSquare(vec::Vec3 p, Segment s, float* t)
	{
		vec::Vec3 result(SegPointLatestPoint(p, s, t));

		return vec::Vec3::LengthSquare(result, p);
	}
};
