#pragma once
#include "../container.h"
#include "../mymath/collision.h"
#include "../aliastemplate.h"

struct AdjacentData
{
	Segment segment;
};


class NavigationMesh
{
public:
	NavigationMesh(const Polygon3D& mesh);
	~NavigationMesh();

	static NavigationMesh* FindPath(at::vet<NavigationMesh>& list, NavigationMesh* start, NavigationMesh* goal, float w);

	NavigationMesh* GetParent() const { return _nParent; }

	void RegisterAdjacent(NavigationMesh* adjacent, const AdjacentData& data);

	void SetBlocked(bool blocked) { _bBlocked = blocked; }

	static void CreateList(const at::vec<Polygon3D>& polygon, at::vec<NavigationMesh>& outlist);

	Polygon3D GetMesh() const { return _mesh; }

	at::vpcc<NavigationMesh*, AdjacentData> GetAdjacentList()& { return _adjacentList; }

	void Clear();

	const vec::Vec3& GetPosition() const { return _pos; }

protected:
	//親
	NavigationMesh* _nParent;

	//メッシュ
	Polygon3D _mesh;

	//隣接リスト
	at::vpcc<NavigationMesh*, AdjacentData> _adjacentList;


	//オープンセット
	bool _bOpenSet;

	//クローズセット
	bool _bCloseSet;

	//位置(ナビメッシュ(道)として使用できるか
	bool _bBlocked;

	//コスト
	float _fCost;

	//位置
	vec::Vec3 _pos;

	//移動コスト
	float _fCostToMove;

	//ゴールまでのコスト
	float _fCostToGoal;
};

