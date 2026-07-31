///*********************************************************************/
//// * \file   mymath.h
//// * \brief  自分用の数学関数
//// *
//// * \author 鈴木裕稀
///*********************************************************************/
//
//#pragma once
//#include "container.h"
//#include "aliastemplate.h"
//
//// 計算用マクロ
//#define	PI	(3.1415926535897932386f)
//#define	DEG2RAD(x)			( ((x) / 180.0f ) * PI )
//#define	RAD2DEG(x)			( ((x) * 180.0f ) / PI )
//#define TWO_PI (PI * 2.0f);
//
//constexpr float GRAVITY_COEFFICIENT = 7.5625f; // 自由落下の係数
//namespace mymath
//{
//	// 3Dブロック情報
//	struct BLOCKPOS
//	{
//		std::string name;	// ブロック名
//		float x, y, z;	// ブロックの位置
//		float rx, ry, rz; // ブロックの回転
//		float sx, sy, sz; // ブロックのスケール
//		int modelHandle; // ブロックのモデルハンドル
//		int drawFrame; // ブロックの描画フレーム
//		int collisionFrame; // ブロックの当たり判定フレーム
//	};
//	extern std::vector<BLOCKPOS> _vBlockPos;
//
//	// 様々なイージング
//	// https://game-ui.net/?p=835 を参考に
//	float EasingLinear(float cnt, float start, float end, float frames);
//	float EasingInQuad(float cnt, float start, float end, float frames);
//	float EasingOutQuad(float cnt, float start, float end, float frames);
//	float EasingInOutQuad(float cnt, float start, float end, float frames);
//	float EasingInCubic(float cnt, float start, float end, float frames);
//	float EasingOutCubic(float cnt, float start, float end, float frames);
//	float EasingInOutCubic(float cnt, float start, float end, float frames);
//	float EasingInQuart(float cnt, float start, float end, float frames);
//	float EasingOutQuart(float cnt, float start, float end, float frames);
//	float EasingInOutQuart(float cnt, float start, float end, float frames);
//	float EasingInQuint(float cnt, float start, float end, float frames);
//	float EasingOutQuint(float cnt, float start, float end, float frames);
//	float EasingInOutQuint(float cnt, float start, float end, float frames);
//	float EasingInSine(float cnt, float start, float end, float frames);
//	float EasingOutSine(float cnt, float start, float end, float frames);
//	float EasingInOutSine(float cnt, float start, float end, float frames);
//	float EasingInExpo(float cnt, float start, float end, float frames);
//	float EasingOutExpo(float cnt, float start, float end, float frames);
//	float EasingInOutExpo(float cnt, float start, float end, float frames);
//	float EasingInCirc(float cnt, float start, float end, float frames);
//	float EasingOutCirc(float cnt, float start, float end, float frames);
//	float EasingInOutCirc(float cnt, float start, float end, float frames);
//	float EasingInElastic(float cnt, float start, float end, float frames);
//	float EasingOutElastic(float cnt, float start, float end, float frames);
//	float EasingInOutElastic(float cnt, float start, float end, float frames);
//	float EasingInBounce(float cnt, float start, float end, float frames);
//	float EasingOutBounce(float cnt, float start, float end, float frames);
//	float EasingInOutBounce(float cnt, float start, float end, float frames);
//	float EasingInBack(float cnt, float start, float end, float frames);
//	void TestEasing();
//
//	// 値をmin～maxの範囲にクランプする関数
//	template<typename T>
//	static T Clamp(T min, T max, T n)
//	{
//		if(n <= min)
//		{
//			return min;
//		}
//		if(n >= max)
//		{
//			return max;
//		}
//
//		return n;
//	}
//}
