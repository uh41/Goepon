#pragma once
#include "navigationmesh.h"

namespace navi
{
	constexpr auto VERTICAL_DIS = 1000.0f; // ナビメッシュ探索時の垂直方向のオフセット距離
}

class Navi
{
public:
	Navi();
	~Navi();

	void CreateNavigtionMesh();

	bool FindPath(const vec::Vec3& startpos, const vec::Vec3& goalpos, at::vec<vec::Vec3>& path, float radius = 0.0f);

	auto GetHandle() const { return _iHandle;}

	bool LoadModel(const char* fliename);

	at::vec<NavigationMesh>& GetNavigationMesh() { return _nNavigationMesh; }

private:
	void GetPolygonData(int handle, at::vec<Polygon3D>& outpolygon);

protected:
	int _iHandle;

	at::vec<NavigationMesh> _nNavigationMesh;

};

