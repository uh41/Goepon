#include "collisionmanager.h"

CollisionManager* CollisionManager::GetInstance()
{
	static CollisionManager instance;
	return &instance;
}

CollisionManager::CollisionManager()
{
	_debugDraw = false;
}

CollisionManager::~CollisionManager()
{

}

bool CollisionManager::Initialize()
{
	_debugDraw = false;
	_debugInfo.hasData = false;

	return true;
}

bool CollisionManager::Terminate()
{
	return true;
}

bool CollisionManager::CheckSectorToPosition(
	const vec::Vec3& secpos,
	const vec::Vec3& secdir,
	float secrad,
	float sechalfangle,
	const vec::Vec3& targetpos
)
{
	// デバック情報を記録
	_debugInfo.sector.pos = secpos;
	_debugInfo.sector.dir = secdir;
	_debugInfo.sector.rad = secrad;
	_debugInfo.sector.halfAngle = sechalfangle;
	_debugInfo.pos = targetpos;
	_debugInfo.hasData = true;

	// 扇形の中心から対象位置のベクトル
	vec::Vec3 target = vec3::VSub(targetpos, secpos);
	target.y = 0.0f; // Y軸を無視

	// 距離をチェック
	float distSq = target.LengthSquare();
	// 半径より遠い場合は当たらない
	if(distSq > (secrad * secrad))
	{
		_debugInfo.isResult = false;
		return false;
	}

	// 扇形の向きを正規化
	vec::Vec3 sectorDir = secdir;
	sectorDir.y = 0.0f; // Y軸を無視
	// 正規化
	if(vec3::VSize(sectorDir) < 0.01f)
	{
		_debugInfo.isResult = false;
		return false;
	}
	sectorDir = vec3::VNorm(sectorDir);

	// 対象の方向を正規化
	vec::Vec3 targetDir = vec3::VNorm(target);

	// 内積を計算
	float cosTheta = cosf(sechalfangle);// 扇形の半角のcos値
	float dot = vec::Vec3::Dot(sectorDir, targetDir);// 内積

	bool hit = (dot >= cosTheta);// 内積がcos値以上なら当たり
	_debugInfo.isResult = hit;// 当たり判定結果をデバック情報に記録

	return hit;
}

void CollisionManager::RenderDebug(unsigned int r, unsigned int g, unsigned int b)
{
	// デバック描画が無効、またはデバック情報が無い場合は何もしない
	if(!_debugDraw || !_debugInfo.hasData)
	{
		return;
	}

	SectorCollison& sector = _debugInfo.sector;
	CircleCollison& circle = _debugInfo.circle;
	bool hit = _debugInfo.isResult;

	// 扇形のデバック描画
	unsigned int secColor = GetColor(r, g, b);
	unsigned int cirColor = GetColor(r, g, b);

	// 扇形の描画
	int segment = 32;
	vec::Vec3 sectorDir = sector.dir;
	sectorDir.y = 0.0f;

	float centerAngle = atan2f(sectorDir.z, sectorDir.x);
	float a1 = centerAngle - sector.halfAngle;
	float a2 = centerAngle + sector.halfAngle;

	// 扇形の辺の点を計算
	vec::Vec3 pos = vec3::VGet(
		sector.pos.x + cosf(a1) * sector.rad,
		sector.pos.y,
		sector.pos.z + sinf(a1) * sector.rad
	);
}