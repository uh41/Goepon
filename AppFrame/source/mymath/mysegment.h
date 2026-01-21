/*********************************************************************/
// * \file   mysegment.h
// * \brief  線分（始点と終点がある線)
// *
// * \author 鈴木裕稀
// * \date   2026/1/11
// * \作業内容: 新規作成 鈴木裕稀　2026/1/11
/*********************************************************************/

#pragma once
#include "vector3.h"

class Segment
{
public:
	vec::Vec3 start;
	vec::Vec3 end;

	Segment()
	{
		start = vec::Vec3(0.0f, 0.0f, 0.0f);
		end = vec::Vec3(0.0f, 0.0f, 0.0f);
	}
	Segment(const vec::Vec3& start, const vec::Vec3& end)
	{
		this->start = start;
		this->end = end;
	}
	Segment(const Segment& segment)
	{
		*this = segment;
	}

	// 線分のベクトル
	vec::Vec3 Vector()
	{
		return end - start;
	}

	// 線分のベクトルの長さ
	float Length()
	{
		return this->Vector().Length();
	}

	// 線分のベクトルの長さの2乗
	float LengthSpuare()
	{
		return this->Vector().LengthSquare();
	}

	// 線形補間 (0.0 ～ 1.0)
	vec::Vec3 LinearInterpolation(const float& t)
	{
		return start + vec::Vec3(end - start) * t;
	}

	// 二つの線分が等しいかどうか
	static bool IsSame(Segment s1, Segment s2)
	{
		if(s1.start == s2.start && s1.end == s2.end)
		{
			return true;
		}
		return false;
	}

	// 二つの線分が重なっているかどうか
	static bool IsOverLap(Segment s1, Segment s2)
	{
		if(vec::Vec3i::LengthSquare(s1.start, s1.end) == vec::Vec3i::LengthSquare(s2.start, s2.end))
		{
			if(s1.start == s2.start || s1.start == s2.end)
			{
				if(s1.end == s2.start || s1.end == s2.end)
				{
					return true;
				}
			}
		}
		return false;
	}
};

