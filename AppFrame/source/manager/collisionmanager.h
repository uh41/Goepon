/*********************************************************************/
// * \file   CollisionManager.h
// * \brief  コリジョン判定マネージャークラス(扇形と円)
// *
// * \author 鈴木裕稀
// * \date   2026/01/23
// * \作業内容: 新規作成 鈴木裕稀　2026/01/23
/*********************************************************************/


#pragma once
#include "../mymath/vectorconverter.h"

class CollisionManager
{
public:
	// 円の当たり判定構造体
	struct CircleCollison
	{
		vec::Vec3 pos;		// 円の中心座標
		float rad;			// 円の半径

		CircleCollison()
		{
			pos = vec::Vec3(0.0f, 0.0f, 0.0f);
			rad = 0.0f;
		}

		CircleCollison(const vec::Vec3& p, float r)
		{
			pos = p;
			rad = r;
		}
	};

	// 扇形の当たり判定構造体
	struct SectorCollison
	{
		vec::Vec3 pos;		// 扇形の中心座標
		vec::Vec3 dir;		// 扇形の向き(正規化ベクトル)
		float rad;			// 扇形の半径
		float halfAngle;	// 扇形の半角(ラジアン)

		SectorCollison()
		{
			pos = vec::Vec3(0.0f, 0.0f, 0.0f);
			dir = vec::Vec3(0.0f, 0.0f, 1.0f);
			rad = 0.0f;
			halfAngle = 0.0f;
		}

		SectorCollison(const vec::Vec3& p, const vec::Vec3& d, float r, float angle)
		{
			pos = p;
			dir = d;
			rad = r;
			halfAngle = angle;
		}
	};

	// デバック情報構造体
	struct DebugInfo
	{
		SectorCollison sector;	// 扇形の当たり判定情報
		CircleCollison circle;	// 円の当たり判定情報
		bool isResult;		// 当たり判定結果
		bool hasData;	// デバック情報が存在するかどうか
		vec::Vec3 pos;// ターゲットの位置

		DebugInfo()
		{
			isResult = false;
			hasData = false;
			pos = vec::Vec3(0.0f, 0.0f, 0.0f);
		}
	};

	static CollisionManager* GetInstance();

	bool Initialize();
	bool Terminate();

	// 扇形と位置の当たり判定
	// \param secpos 扇形の中心位置
	// \param secdir 扇形の向き
	// \param secrad 扇形の半径
	// \param sechalfangle 扇形の半角(ラジアン)
	// \param targetpos 対象の位置
	// \return 当たっている場合true
	bool CheckSectorToPosition(
		const vec::Vec3& secpos,
		const vec::Vec3& secdir,
		float secrad,
		float sechalfangle,
		const vec::Vec3& targetpos
	);

	//// 円と位置の当たり判定
	//// \param circlepos 円の中心位置
	//// \param circlerad 円の半径
	//// \param targetpos 対象の位置
	//// \return 当たっている場合true
	//bool CheckCircleToPosition(
	//	const vec::Vec3& cirpos,
	//	float cirrad,
	//	const vec::Vec3& targetpos
	//);

	//// 円と円の当たり判定
	//// \param cir1pos 円1の中心位置
	//// \param cir1rad 円1の半径
	//// \param cir2pos 円2の中心位置
	//// \param cir2rad 円2の半径
	//// \return 当たっている場合true
	//bool CheckCircleToCircle(
	//	const vec::Vec3& cir1pos,
	//	float cir1rad,
	//	const vec::Vec3& cir2pos,
	//	float cir2rad
	//);

	void SetDebugDraw(bool enable) { _debugDraw = enable; }
	bool GetDebugDraw() const { return _debugDraw; }

	void RenderDebug(unsigned int r, unsigned int g, unsigned int b);

	const DebugInfo& GetDebugInfo() const { return _debugInfo; }

private:
	CollisionManager();
	~CollisionManager();

	// コピー禁止
	CollisionManager(const CollisionManager&) = delete;
	CollisionManager& operator=(const CollisionManager&) = delete;

	bool _debugDraw;			// デバック描画を行うかどうか
	DebugInfo _debugInfo;		// デバック情報
};

