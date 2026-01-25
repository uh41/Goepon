#include "collisionmanager.h"
#include "../aliastemplate.h"
#include "../mymath.h"

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
    //float halfangleclamp = std::clamp(sechalfangle, 0.0f, PI);
    float halfangleclamp = mymath::Clamp(0.0f, PI, sechalfangle);
    float cosTheta = cosf(sechalfangle);// 扇形の半角のcos値
    float dot = vec::Vec3::Dot(sectorDir, targetDir);// 内積
	dot = mymath::Clamp(-1.0f, 1.0f, dot);// 内積を-1～1にクランプ

    bool hit = (dot >= cosTheta);// 内積がcos値以上なら当たり
    _debugInfo.isResult = hit;// 当たり判定結果をデバック情報に記録

    return hit;
}

// 円と位置の当たり判定
bool CollisionManager::CheckCircleToPosition(
    const vec::Vec3& cirpos,
    float cirrad,
    const vec::Vec3& targetpos
)
{
	vec::Vec3 target = vec3::VSub(targetpos, cirpos);// 円の中心から対象位置へのベクトル
	target.y = 0.0f; // Y軸を無視
	float distSq = target.LengthSquare();
	bool hit = (distSq <= (cirrad * cirrad));       // 半径以内なら当たり

	_debugInfo.circle1.pos = cirpos;
	_debugInfo.circle1.rad = cirrad;
	_debugInfo.pos = targetpos;
	_debugInfo.isResult = hit;
	_debugInfo.hasData = true;

	return hit;
}

bool CollisionManager::CheckCircleToCircle(
    const vec::Vec3& cir1pos,
    float cir1rad,
    const vec::Vec3& cir2pos,
    float cir2rad
)
{
	vec::Vec3 cirtocirpos = vec3::VSub(cir2pos, cir1pos);// 円1の中心から円2の中心へのベクトル
	cirtocirpos.y = 0.0f; // Y軸を無視
	float distSq = cirtocirpos.LengthSquare();
	float radd = cir1rad + cir2rad;     // 半径の和
	bool hit = (distSq <= (radd * radd)); // 半径の和以内なら当たり

	_debugInfo.circle1.pos = cir1pos;
	_debugInfo.circle1.rad = cir1rad;
	_debugInfo.circle2.pos = cir2pos;
	_debugInfo.circle2.rad = cir2rad;
	_debugInfo.isResult = hit;
	_debugInfo.hasData = true;

	return hit;
}

bool CollisionManager::CheckSectorToCapsule(
    const vec::Vec3& secpos,
    const vec::Vec3& secdir,
    float secrad,
    float sechalfangle,
    const vec::Vec3& captop,
    const vec::Vec3& capbottom,
    float caprad
)
{
    vec::Vec3 sec = secpos;
	vec::Vec3 top = captop;
	vec::Vec3 bottom = capbottom;
	sec.y = 0.0f;    // Y軸を無視
	top.y = 0.0f;    // Y軸を無視
	bottom.y = 0.0f; // Y軸を無視

	vec::Vec3 toptobottom = vec3::VSub(bottom, top);
	float tbcLenSq = toptobottom.LengthSquare();

    vec::Vec3 capcenter = top;
	// カプセルの中心位置を計算
	float numerator = vec::Vec3::Dot(vec3::VSub(sec, top), toptobottom);// 分子
    float t;
	// tbcLenSqが0の場合、カプセルの上端と下端が同じ位置にあるのでtを0にする
    if(tbcLenSq > 0.0f)
    {
		t = numerator / tbcLenSq;
    }
    else
    {
        t = 0.0f;
	}

	t = mymath::Clamp(0.0f, 1.0f, t);// 0～1にクランプ

	vec::Vec3 closest = top + toptobottom * t;// カプセルの中心に最も近い点

	// 扇形とカプセル中心の当たり判定を行う
    bool hit = CheckSectorToPosition(
        sec,
        secdir,
        secrad + caprad, // カプセルの半径分を加算
        sechalfangle,
        closest
	);
    // デバック情報を記録
    _debugInfo.sector.pos = secpos;
    _debugInfo.sector.dir = secdir;
    _debugInfo.sector.rad = secrad;
    _debugInfo.sector.halfAngle = sechalfangle;
    _debugInfo.circle1.pos = closest;
    _debugInfo.circle1.rad = caprad;
    _debugInfo.isResult = hit;
    _debugInfo.hasData = true;
	return hit;
}

bool CollisionManager::CheckPositionToMV1Collision(
    const vec::Vec3& pos,
    int handle,
    int frameindex,
    float colsuby,
    vec::Vec3& hitpos
)
{
	// 開始位置を設定
	vec::Vec3 start = vec3::VAdd(pos, vec3::VGet(0.0f, colsuby, 0.0f));

    vec::Vec3 end = vec3::VAdd(pos, vec3::VGet(0.0f, -9999.0f, 0.0f));

    MV1_COLL_RESULT_POLY hitpoly = DxlibConverter::MV1CollCheckLine(
        handle,
        frameindex,
        start,
        end
	);

    if(hitpoly.HitFlag)
    {
		hitpos = vec3::VGet(pos.x, hitpoly.HitPosition.y, pos.z);

		_debugInfo.pos = hitpos;
        _debugInfo.hasData = true;
		_debugInfo.isResult = true;
        return true;
    }

	_debugInfo.hasData = true;
    _debugInfo.isResult = false;
	return false;
}

void CollisionManager::RenderDebug(unsigned int r, unsigned int g, unsigned int b)
{
    // デバック描画が無効、またはデバック情報が無い場合は何もしない
    if(!_debugDraw || !_debugInfo.hasData)
    {
        return;
    }

    SectorCollison& sector = _debugInfo.sector;
    CircleCollison& circle = _debugInfo.circle1;
    bool hit = _debugInfo.isResult;

    circle.pos = _debugInfo.pos;

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

    // 扇形の円弧を描画
    for(int i = 0; i <= segment; ++i)
    {
        float angle = a1 + (a2 - a1) * (static_cast<float>(i) / static_cast<float>(segment));
        // 円弧上の現在の点
        vec::Vec3 curPos = vec3::VGet(
            sector.pos.x + cosf(angle) * sector.rad,
            sector.pos.y,
            sector.pos.z + sinf(angle) * sector.rad
        );
        DxlibConverter::DrawLine3D(pos, curPos, secColor);
        pos = curPos;
    }

    // 扇形の両端の線を描画
    vec::Vec3 leftEdge = vec3::VGet(
        sector.pos.x + cosf(a1) * sector.rad,
        sector.pos.y,
        sector.pos.z + sinf(a1) * sector.rad
    );
    vec::Vec3 rightEdge = vec3::VGet(
        sector.pos.x + cosf(a2) * sector.rad,
        sector.pos.y,
        sector.pos.z + sinf(a2) * sector.rad
    );

    DxlibConverter::DrawLine3D(sector.pos, leftEdge, secColor);
    DxlibConverter::DrawLine3D(sector.pos, rightEdge, secColor);

    // 対象位置から扇形中心への線を描画
    DxlibConverter::DrawLine3D(sector.pos, circle.pos, cirColor);

    // 円弧の輪郭を見やすくするために少し上に描画
    float upY = 10.0f;
    vec::Vec3 posUp = vec3::VGet(
        sector.pos.x + cosf(a1) * sector.rad,
        sector.pos.y + upY,
        sector.pos.z + sinf(a1) * sector.rad
    );

    for(int i = 0; i <= segment; i++)
    {
        float angle = a1 + (a2 - a1) * (static_cast<float>(i) / static_cast<float>(segment));
        // 円弧上の現在の点
        vec::Vec3 curPosUp = vec3::VGet(
            sector.pos.x + cosf(angle) * sector.rad,
            sector.pos.y + upY,
            sector.pos.z + sinf(angle) * sector.rad
        );
        DxlibConverter::DrawLine3D(posUp, curPosUp, secColor);
		posUp = curPosUp;
    }


    // 上側の両端の線を描画
    vec::Vec3 leftEdgeUp = vec3::VGet(
        sector.pos.x + cosf(a1) * sector.rad,
        sector.pos.y + upY,
        sector.pos.z + sinf(a1) * sector.rad
    );
    vec::Vec3 rightEdgeUp = vec3::VGet(
        sector.pos.x + cosf(a2) * sector.rad,
        sector.pos.y + upY,
        sector.pos.z + sinf(a2) * sector.rad
    );
    DxlibConverter::DrawLine3D(sector.pos + vec3::VGet(0.0f, upY, 0.0f), leftEdgeUp, secColor);
    DxlibConverter::DrawLine3D(sector.pos + vec3::VGet(0.0f, upY, 0.0f), rightEdgeUp, secColor);

    // 対象位置の周辺にマーカーを描画

    at::art<vec::Vec3, 4> offsetpos =
    {
        vec3::VGet(-10.0f, 5.0f, 0.0f),
        vec3::VGet(10.0f, 5.0f, 0.0f),
        vec3::VGet(0.0f, 5.0f, -10.0f),
        vec3::VGet(0.0f, 5.0f, 10.0f),
    };

	// マーカー位置を計算
	at::art<vec::Vec3, 4> markerpos;
    for(size_t i = 0; i < offsetpos.size(); i++)
    {
        markerpos[i] = vec3::VAdd(circle.pos, offsetpos[i]);
    }

	// 十字を描画
    DxlibConverter::DrawLine3D(markerpos[0], markerpos[1], cirColor);
	DxlibConverter::DrawLine3D(markerpos[2], markerpos[3], cirColor);

    if(hit)
    {
		unsigned int hitColor = GetColor(r, g, b);
        at::art<vec::Vec3, 4> hitoffset =
        {
            vec3::VGet(-6.0f, 8.0f, 0.0f),
            vec3::VGet(6.0f, 8.0f, 0.0f),
            vec3::VGet(0.0f, 8.0f, -6.0f),
            vec3::VGet(0.0f, 8.0f, 6.0f),
        };
        at::art<vec::Vec3, 4> hitmarker;
        for(size_t i = 0; i < hitoffset.size(); i++)
        {
			hitmarker[i] = vec3::VAdd(circle.pos, hitoffset[i]);
        }
		// 十字を描画
        DxlibConverter::DrawLine3D(hitmarker[0], hitmarker[1], hitColor);
		DxlibConverter::DrawLine3D(hitmarker[2], hitmarker[3], hitColor);
    }
}