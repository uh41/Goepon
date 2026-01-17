#pragma once
#include "vector3.h"
#include "mysegment.h"
#include "../container.h"

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
	const vec::Vec3 NormalVector()
	{
		return vec::Vec3::Cross((_vVertex2 - _vVertex1), (_vVertex3 - _vVertex1)).Normalized();
	}

	// 重点(三角形の重心)
	vec::Vec3 Emphasis()
	{
		return vec::Vec3::Emphasis(_vVertex1, _vVertex2, _vVertex3);
	}

	// 拡大縮小
	void SetScale(const float& scale)
	{
		vec::Vec3 emphasis = Emphasis();
		// 線形補間で各頂点を重点に近づける
		_vVertex1 = vec::Vec3::Lerp(_vVertex1, emphasis, scale);
		_vVertex2 = vec::Vec3::Lerp(_vVertex2, emphasis, scale);
		_vVertex3 = vec::Vec3::Lerp(_vVertex3, emphasis, scale);
	}

	// 各頂点の取得
	bool IsConect(Polygon3D& other, Segment* side)
	{
		// 自分の辺
		std::array<Segment, 6> myside =
		{
			Segment(_vVertex1, _vVertex2),
			Segment(_vVertex2, _vVertex1),
			Segment(_vVertex2, _vVertex3),
			Segment(_vVertex3, _vVertex2),
			Segment(_vVertex3, _vVertex1),
			Segment(_vVertex1, _vVertex3)
		};

		// 相手の辺
		std::array<Segment, 3> otherside =
		{
			Segment(other._vVertex1, other._vVertex2),
			Segment(other._vVertex2, other._vVertex3),
			Segment(other._vVertex3, other._vVertex1)
		};

		for(size_t i = 0; i < myside.size() * otherside.size(); i++)
		{
			Segment my = myside[i / otherside.size()];
			Segment ot = otherside[i % otherside.size()];
			if(Segment::IsSame(my, ot))
			{
				if(side)
				{
					*side = my;
				}
				return true;
			}
		}
		return false;
	}

	vec::Vec3 _vVertex1;
	vec::Vec3 _vVertex2;
	vec::Vec3 _vVertex3;
};