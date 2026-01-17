#include "navi.h"
#include "../mymath/vectorconverter.h"

Navi::Navi()
{
	_iHandle = -1;
}

Navi::~Navi()
{

}

void Navi::CreateNavigtionMesh()
{
	at::vec<Polygon3D> polygon;
	GetPolygonData(_iHandle, polygon);
	NavigationMesh::CreateList(polygon, _nNavigationMesh);
}

bool Navi::FindPath(const vec::Vec3& startpos, const vec::Vec3& goalpos, at::vec<vec::Vec3>& path, float radius)
{
	NavigationMesh* start = nullptr;
	NavigationMesh* goal = nullptr;

	// 開始地点と終了地点のナビメッシュを探す
	for(auto&& navmesh : _nNavigationMesh)
	{
		// 開始地点
		if(HitCheck_Line_Triangle(
			VectorConverter::VecToDxLib(startpos + vec::Vec3(0.0f, 1.0f, 0.0f) * navi::VERTICAL_DIS),	// 上から
			VectorConverter::VecToDxLib(startpos + vec::Vec3(0.0f, -1.0f, 0.0f) * navi::VERTICAL_DIS),	// 下まで
			VectorConverter::VecToDxLib(navmesh.GetMesh()._vVertex1),									// 三角形の頂点１
			VectorConverter::VecToDxLib(navmesh.GetMesh()._vVertex2),									// 三角形の頂点２
			VectorConverter::VecToDxLib(navmesh.GetMesh()._vVertex3)									// 三角形の頂点３
		).HitFlag == 1)
		{
			start = &navmesh;		// 見つけた
		}

		// 終了地点
		if(HitCheck_Line_Triangle(
			VectorConverter::VecToDxLib(goalpos + vec::Vec3(0.0f, 1.0f, 0.0f) * navi::VERTICAL_DIS),	// 上から
			VectorConverter::VecToDxLib(goalpos + vec::Vec3(0.0f, -1.0f, 0.0f) * navi::VERTICAL_DIS),	// 下まで
			VectorConverter::VecToDxLib(navmesh.GetMesh()._vVertex1),									// 三角形の頂点１
			VectorConverter::VecToDxLib(navmesh.GetMesh()._vVertex2),									// 三角形の頂点２
			VectorConverter::VecToDxLib(navmesh.GetMesh()._vVertex3)									// 三角形の頂点３
		).HitFlag == 1)
		{
			goal = &navmesh;		// 見つけた
		}

		if(start && goal)
		{
			break;
		}
	}

	// 開始地点と終了地点のナビメッシュが見つからなかった
	if(!start || !goal)
	{
		return false;
	}

	auto nodepath = NavigationMesh::FindPath(_nNavigationMesh, start, goal, radius);	// ナビメッシュの経路探索

	// 経路が見つからなかった
	if(!nodepath)
	{
		return false;
	}

	// ナビメッシュのfindpathで帰ってきたのはゴール地点になるため開始地点を後ろに追加したあとで、逆順にする
	path.emplace_back(startpos);				// 開始地点を追加

	std::reverse(path.begin(), path.end());		// 逆順にする

	path.emplace_back(goalpos);					// 終了地点を追加

	return true;
}

bool Navi::LoadModel(const char* fliename)
{
	_iHandle = MV1LoadModel(fliename);

	if(_iHandle > -1)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void Navi::GetPolygonData(int handle, at::vec<Polygon3D>& polygon)
{
	// モデルに含まれる情報bを取得
	MV1_REF_POLYGONLIST list = MV1GetReferenceMesh(handle, 0, TRUE);

	//　頂点のインデックスを取得
	MV1_REF_POLYGON* dxPolygon = list.Polygons;
	// 頂点の座標を取得
	MV1_REF_VERTEX* dxVertex = list.Vertexs;

	// ポリゴン数分ループ
	for(int i = 0; i < list.PolygonNum; i++)
	{
		Polygon3D myPolygon
		(
			VectorConverter::DxLibToVec(dxVertex[dxPolygon[i].VIndex[0]].Position),		// 頂点１
			VectorConverter::DxLibToVec(dxVertex[dxPolygon[i].VIndex[1]].Position),		// 頂点２
			VectorConverter::DxLibToVec(dxVertex[dxPolygon[i].VIndex[2]].Position)		// 頂点３
		);
		polygon.emplace_back(myPolygon);
	}
}