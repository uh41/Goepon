#pragma once
#include "vector3operator.h"
#include "../container.h"

class DxlibConverter
{
public:
	// DxLibからVector3へ変換
	static VECTOR VecToDxLib(vec::Vec3 v1)
	{
		return VGet(v1.x, v1.y, v1.z);
	}

	// Vector3からDxLibへ変換
	static vec::Vec3 DxLibToVec(VECTOR v1)
	{
		return vec3::VGet(v1.x, v1.y, v1.z);
	}

	// モデルの特定フレームに対して線分と当たり判定を行う
	static MV1_COLL_RESULT_POLY MV1CollCheckLine(int handle, int frame, vec::Vec3 v1, vec::Vec3 v2)
	{
		return MV1CollCheck_Line(
			handle,
			frame,
			VecToDxLib(v1),
			VecToDxLib(v2)
		);
	}

	// カプセル同士の当たり判定
	static int HitCheckCapsuleToCapsule(vec::Vec3 t1, vec::Vec3 b1, float r1, vec::Vec3 t2, vec::Vec3 b2, float r2)
	{
		return HitCheck_Capsule_Capsule(
			VecToDxLib(t1),
			VecToDxLib(b1),
			r1,
			VecToDxLib(t2),
			VecToDxLib(b2),
			r2
		);
	}

	// 3点で構成される三角形と線分の当たり判定
	static HITRESULT_LINE HitCheckLineToTriangle(vec::Vec3 linepos1, vec::Vec3 linepos2, vec::Vec3 triV1, vec::Vec3 triV2, vec::Vec3 triV3)
	{
		return HitCheck_Line_Triangle(
			VecToDxLib(linepos1),
			VecToDxLib(linepos2),
			VecToDxLib(triV1),
			VecToDxLib(triV2),
			VecToDxLib(triV3)
		);
	}

	static int DrawLine3D(vec::Vec3 v1, vec::Vec3 v2, unsigned int color)
	{
		return ::DrawLine3D(
			VecToDxLib(v1),
			VecToDxLib(v2),
			color
		);
	}

	static int SetShadowMapDrawArea(int handle, vec::Vec3 min, vec::Vec3 max)
	{
		return ::SetShadowMapDrawArea(
			handle,
			VecToDxLib(min),
			VecToDxLib(max)
		);
	}
	
};
