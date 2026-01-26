/*********************************************************************/
// * \file   mymath.h
// * \brief  自分用の数学関数
// *
// * \author 鈴木裕稀
// * \date   2025/12/15
// * \作業内容: 新規作成 鈴木裕稀　2025/12/15
/*********************************************************************/

#pragma once
#include "container.h"

// 計算用マクロ
#define	PI	(3.1415926535897932386f)
#define	DEG2RAD(x)			( ((x) / 180.0f ) * PI )
#define	RAD2DEG(x)			( ((x) * 180.0f ) / PI )

namespace mymath
{

	// 当たり判定用構造体
	struct COLLISIONCAPSULE
	{
		VECTOR underPos;		// カプセルの下端座標
		VECTOR overPos;			// カプセルの上端座標
		float r;				// カプセルの半径
		int modelHandle;		// 追従させるモデルのハンドル
		int frameNum;			// 追従させるモデルのフレーム番号
	};

	// 攻撃判定用構造体
	struct ATTACKCOLLISION
	{
		COLLISIONCAPSULE capsule; // 攻撃判定用カプセル
		COLLISIONCAPSULE oldCapsule; // 1フレーム前の攻撃判定用カプセル
		std::string attackChara; // 攻撃をしたキャラクターの名前
		MATRIX modelId; // 1フレーム前の追従させるモデルのフレーム変換行列
		VECTOR vector;	// 攻撃の向き
		float damage;	// ダメージ量
		int waitTime;	// 攻撃が有効になるまでの時間
		int activeTime;	// 攻撃が有効な時間
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

	// 3Dブロック情報
	struct BLOCKPOS
	{
		std::string name;	// ブロック名
		float x, y, z;	// ブロックの位置
		float rx, ry, rz; // ブロックの回転
		float sx, sy, sz; // ブロックのスケール
		int modelHandle; // ブロックのモデルハンドル
		int drawFrame; // ブロックの描画フレーム
	};
	// Declare the vector as extern to avoid multiple definitions
	extern std::vector<BLOCKPOS> _vBlockPos;

	// 様々なイージング
	// https://game-ui.net/?p=835 を参考に
	float EasingLinear(float cnt, float start, float end, float frames);
	float EasingInQuad(float cnt, float start, float end, float frames);
	float EasingOutQuad(float cnt, float start, float end, float frames);
	float EasingInOutQuad(float cnt, float start, float end, float frames);
	float EasingInCubic(float cnt, float start, float end, float frames);
	float EasingOutCubic(float cnt, float start, float end, float frames);
	float EasingInOutCubic(float cnt, float start, float end, float frames);
	float EasingInQuart(float cnt, float start, float end, float frames);
	float EasingOutQuart(float cnt, float start, float end, float frames);
	float EasingInOutQuart(float cnt, float start, float end, float frames);
	float EasingInQuint(float cnt, float start, float end, float frames);
	float EasingOutQuint(float cnt, float start, float end, float frames);
	float EasingInOutQuint(float cnt, float start, float end, float frames);
	float EasingInSine(float cnt, float start, float end, float frames);
	float EasingOutSine(float cnt, float start, float end, float frames);
	float EasingInOutSine(float cnt, float start, float end, float frames);
	float EasingInExpo(float cnt, float start, float end, float frames);
	float EasingOutExpo(float cnt, float start, float end, float frames);
	float EasingInOutExpo(float cnt, float start, float end, float frames);
	float EasingInCirc(float cnt, float start, float end, float frames);
	float EasingOutCirc(float cnt, float start, float end, float frames);
	float EasingInOutCirc(float cnt, float start, float end, float frames);

	template<typename T>
	static T Clamp(T min, T max, T n)
	{
		if(n <= min)
		{
			return min;
		}
		if(n >= max)
		{
			return max;
		}

		return n;
	}
}
