#pragma once
// 計算用マクロ
#define	PI	(3.1415926535897932386f)
#define	DEG2RAD(x)			( ((x) / 180.0f ) * PI )
#define	RAD2DEG(x)			( ((x) * 180.0f ) / PI )

namespace mymath
{

	// 当たり判定用構造体
	struct COLLISIONCAPSULE
	{
		VECTOR underpos;		// カプセルの下端座標
		VECTOR overpos;			// カプセルの上端座標
		float r;				// カプセルの半径
		int modelhandle;		// 追従させるモデルのハンドル
		int framenum;			// 追従させるモデルのフレーム番号
	};

	// 攻撃判定用構造体
	struct ATTACKCOLLISION
	{
		COLLISIONCAPSULE capsule; // 攻撃判定用カプセル
		COLLISIONCAPSULE oldcapsule; // 1フレーム前の攻撃判定用カプセル
		std::string attackChara; // 攻撃をしたキャラクターの名前
		MATRIX modelId; // 1フレーム前の追従させるモデルのフレーム変換行列
		VECTOR vector;	// 攻撃の向き
		float damage;	// ダメージ量
		int waittime;	// 攻撃が有効になるまでの時間
		int activetime;	// 攻撃が有効な時間
		bool follow;	// カメラ方向に追従するかどうか
		bool isHit = false;		// 当たったかどうか
	};


	// AABB(軸に平行な境界ボックス)構造体
	struct AABB
	{
		VECTOR min;
		VECTOR max;
	};

	// 球とAABBの当たり判定
	static inline bool IsHitSphereAABB(const VECTOR& center, float radius, const AABB& box)
	{

		VECTOR closest;// 一番近い点
		// x軸方向
		if(center.x < box.min.x)
		{
			closest.x = box.min.x;
		}
		else if(center.x > box.max.x)
		{
			closest.x = box.max.x;
		}
		else
		{
			closest.x = center.x;
		}

		// y軸方向
		if(center.y < box.min.y)
		{
			closest.y = box.min.y;
		}
		else if(center.y > box.max.y)
		{
			closest.y = box.max.y;
		}
		else
		{
			closest.y = center.y;
		}

		// z軸方向
		if(center.z < box.min.z)
		{
			closest.z = box.min.z;
		}
		else if(center.z > box.max.z)
		{
			closest.z = box.max.z;
		}
		else
		{
			closest.z = center.z;
		}
		float dx = center.x - closest.x;
		float dy = center.y - closest.y;
		float dz = center.z - closest.z;
		float dist2 = dx * dx + dy * dy + dz * dz;

		return dist2 <= radius * radius;
	}

	// 角度を-rad..radの範囲に丸める
	inline float NormalizeAngle(float a)
	{
		float TWO_PI = PI * 2.0f;
		// a を -PI .. PI の範囲にする
		a = fmodf(a + PI, TWO_PI);
		if(a < 0.0f)
		{
			a += TWO_PI;
		}
		return a - PI;
	}
}
