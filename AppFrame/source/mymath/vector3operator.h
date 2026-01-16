#pragma once
#include "vector3.h"

namespace vec3
{
	using Vec3 = vec::Vec3;

	// ベクトルの生成
	template <typename T>
	inline Vec3 VGet(T x, T y, T z)
	{
		return Vec3(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z));
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
	inline Vec3 VScale(const Vec3& v, T scale)
	{
		return v * static_cast<float>(scale);
	}

	// ベクトルのスカラー除算
	template <typename T>
	inline Vec3 VDiv(const Vec3& v, T div)
	{
		return v / static_cast<float>(div);
	}

	// ベクトルの長さ
	inline float VSize(const Vec3& v)
	{
		return v.Length();
	}

	// 正規化
	inline Vec3 VNorm(const Vec3& v)
	{
		return v.Normalized();
	}
}