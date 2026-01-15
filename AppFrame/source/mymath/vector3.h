#pragma once
#include "../container.h"
#include "../mymath.h"

template <typename T>
class Vector3
{
public:
	// DxLibのVECTOR構造体のようにするのでpublicのメンバ変数にする
	T x, y, z;

	Vector3() { x = T(0); y = T(0); z = T(0); }
	Vector3(T x, T y, T z)
	{
		this->x = x;
		this->y = y;
		this->z = z;
	}
	Vector3(const Vector3<T>&) = default;		// コピーコンストラクタ
	
	template <typename U>
	Vector3(const Vector3<U>& v)				// 型変換用のコンストラクタ
	{ 
		this->x = static_cast<T>(v.x);
		this->y = static_cast<T>(v.y);
		this->z = static_cast<T>(v.z);
	}

	~Vector3() = default ;						// デストラクタ

	// 値の代入
	const Vector3<T>& operator = (const Vector3<T>& other)
	{
		this->x = other.x;
		this->y = other.y;
		this->z = other.z;
		return *this;
	}

	// ベクトルの加算代入
	const Vector3<T>& operator += (const Vector3<T>& add)
	{
		this->x += add.x;
		this->y += add.y;
		this->z += add.z;
		return *this;
	}

	// ベクトルの減算代入
	const Vector3<T>& operator -= (const Vector3<T>& sub)
	{
		this->x -= sub.x;
		this->y -= sub.y;
		this->z -= sub.z;
		return *this;
	}

	// ベクトルの乗算代入
	const Vector3<T>& operator *= (const T mul)
	{
		this->x *= mul;
		this->y *= mul;
		this->z *= mul;
		return *this;
	}

	// ベクトルの除算代入
	const Vector3<T>& operator /= (const T div)
	{
		this->x /= div;
		this->y /= div;
		this->z /= div;
		return *this;
	}

	// ベクトルの加算
	friend Vector3<T> operator + (const Vector3<T>& left, const Vector3<T>& right)
	{
		return Vector3<T>(left.x + right.x, left.y + right.y, left.z + right.z);
	}

	// ベクトルの減算
	friend Vector3<T> operator - (const Vector3<T>& left, const Vector3<T>& right)
	{
		return Vector3<T>(left.x - right.x, left.y - right.y, left.z - right.z);
	}

	// ベクトルの乗算
	friend Vector3<T> operator * (const Vector3<T>& left, const T right)
	{
		return Vector3<T>(left.x * right, left.y * right, left.z * right);
	}

	// ベクトルの除算
	friend Vector3<T> operator / (const Vector3<T>& left, const T right)
	{
		return Vector3<T>(left.x / right, left.y / right, left.z / right);
	}

	// 二つのベクトルが等しいかどうか
	bool operator == (const Vector3<T>& other) const
	{
		if(std::abs(this->x - other.x) > static_cast<T>(0.01)) { return false; }
		if(std::abs(this->y - other.y) > static_cast<T>(0.01)) { return false; }
		if(std::abs(this->z - other.z) > static_cast<T>(0.01)) { return false; }
		return true;
	}

	// 二つのベクトルが等しくないかどうか
	bool operator != (const Vector3<T>& other) const
	{
		if(std::abs(this->x - other.x) > static_cast<T>(0.0)) { return true; }
		if(std::abs(this->y - other.y) > static_cast<T>(0.0)) { return true; }
		if(std::abs(this->z - other.z) > static_cast<T>(0.0)) { return true; }
		return false;
	}

	// どちらかのベクトルの方が大きいかどうか(原点から)
	bool operator > (const Vector3<T>&other) const
	{
		return this->LengthSquare() > LengthSquare(Vector3<T>(), other);
	}
	bool operator < (const Vector3<T>&other) const
	{
		return this->LengthSquare() < LengthSquare(Vector3<T>(), other);
	}

	// 内積
	// ２つのベクトルの内積
	static const T Dot(const Vector3<T>&v1, const Vector3<T>&v2)
	{
		return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
	}
	// ベクトル自身との内積
	const T Dot(const Vector3<T>&v) const
	{
		return Dot(*this, v);
	}

	// 外積
	// ２つのベクトルの外積
	static const Vector3<T> Cross(const Vector3<T>&v1, const Vector3<T>&v2)
	{
		Vector3 result;
		result.x = v1.y * v2.z - v1.z * v2.y;
		result.y = v1.z * v2.x - v1.x * v2.z;
		result.z = v1.x * v2.y - v1.y * v2.x;
		return result;
	}
	// ベクトル自身との外積
	const Vector3<T> Cross(const Vector3<T>&v) const
	{
		return Cross(*this, v);
	}

	// ベクトルの長さ
	// ベクトル自身の長さ
	const T Length() const
	{
		return sqrt(Dot(*this, *this));
	}
	// ２点間の距離
	static T Length(const Vector3<T>& v1, const Vector3<T>& v2)
	{
		return Vector3<T>(v1 - v2).Length();
	}

	// ベクトルの長さの二乗
	// ベクトル自身の長さの二乗
	const T LengthSquare() const
	{
		return Dot(*this, *this);
	}
	// ２点間の距離の二乗
	static T LengthSquare(const Vector3<T>& v1, const Vector3<T>& v2)
	{
		return Vector3<T>(v1 - v2).LengthSquare();
	}

	// 内積を使用した角度の算出　度数法で返すのであれば、最後にtrueを入れる
	static T DotAngle(Vector3<T> A, Vector3<T> B, bool isdegree = false)
	{
		// ベクトルの長さが0だと計算できないので注意

		// ベクトルAとBの長さを計算する
		// 内積とベクトルの長さを使ってcosθを求める

		A.Normalize();
		B.Normalize();

		T cossita = Vector3::Dot(A, B);

		// cosθでθを求める
		T sita = acos(cossita);

		if(isdegree)
		{
			sita = RAD2DEG(sita);
		}

		return sita;
	}

	// 外積を使用した角度の算出　度数法で返すのであれば、最後にtrueを入れる
	static T CrossAngleXZ(Vector3<T> A, Vector3<T> B, bool isdegree = false)
	{
		Vector3<T> a(A.x, A.z, T(0));
		Vector3<T> b(B.x, B.z, T(0));

		// ベクトルAとBの長さを計算する
		// 外積とベクトルの長さを使ってsinθを求める

		a.Normalize();
		b.Normalize();

		T sinsita = Vector3::Cross(a, b).z;

		// sinθでθを求める
		T sita = asin(sinsita);

		// ラジアンではなく0 ～ 180度で欲しい場合はコメントを外す
		if(isdegree)
		{
			sita = RAD2DEG(sita);
		}

		return sita;
	}

	// 正規化
	// 正規化されたベクトルを返す
	Vector3<T> Normalized()
	{
		return *this / Length();
	}
	// 自身を正規化する
	void Normalize()
	{
		*this /= Length();
	}

	// 重点
	static Vector3<T> Emphasis(const Vector3<T> ver1, const Vector3<T> ver2, const Vector3<T> ver3)
	{
		return (ver1 + ver2 + ver3) / T(3);
	}

	// 線形補間
	static Vector3<T> Lerp(const Vector3<T>& start, const Vector3<T>& end, T t)
	{
		return start + (end - start) * t;
	}

	// ベクトルの等値判定
	static T Equal(const Vector3<T>& left, const Vector3<T>& right, T dist)
	{
		return LengthSquare(left, right) < (dist * dist);
	}
};

namespace vec
{
	using Vec3 = Vector3<float>;		// Vector3のfloat型
	using Vec3i = Vector3<int>;			// Vector3のint型
	using Vec3d = Vector3<double>;		// Vector3のdouble型
}