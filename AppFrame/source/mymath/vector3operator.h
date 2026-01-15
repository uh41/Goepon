#pragma once
#include "vector3.h"

namespace vec3
{
	using Vec3 = vec::Vec3;

	template <typename T>

	// ベクトルの生成
	inline Vec3 VGet(T x, T y, T z)
	{
		return Vec3(x, y, z);
	}

	// ベクトルの加算
	inline Vec3 VAdd(const Vec3& a, const Vec3& b)
	{
		return a + b;
	}

	// ベクトルの減算
	inline Vec3 VSub(const Vec3& a, const Vec3& b)
	{
		return a - b;
	}

	// ベクトルのスカラー乗算
	template <typename T>
	inline Vec3 VScale(const Vec3& v, T scale )
	{
		return v * scale;
	}

	// ベクトルのスカラー除算
	template <typename T>
	inline Vec3 VDiv(const Vec3& v, T div )
	{
		return v / div;
	}

	// ベクトルの長さ
	template <typename T>
	inline T VSize(const Vec3& v)
	{
		return v.Length();
	}

	// 正規化
	template <typename T>
	inline Vec3 VNorm(const Vec3& v)
	{
		return v.Normalized();
	}

	
}