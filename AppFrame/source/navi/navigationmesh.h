#pragma once
#include "../container.h"
#include "../mymath/collision.h"
#include "../aliastemplate.h"

struct AdjacentData
{
	Segment segment;
};

template <typename T>
class NavigationMesh
{
public:
	NavigationMesh(const Polygon3D& mesh);
	~NavigationMesh();

	static NavigationMesh* FindPath(at::ve<NavigationMesh>& list, NavigationMesh* goal, float w);

	NavigationMesh* GetParent() const { return _nParent; }

	void RegisterAdjacent(NavigationMesh* adjacent, const AdjacentData& data);
};

