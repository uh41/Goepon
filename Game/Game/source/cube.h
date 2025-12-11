#pragma once
#include "objectbase.h"

class Cube : public ObjectBase
{
	typedef ObjectBase base;
public:
	enum class FACE_TYPE
	{
		FRONT,
		Right,
		BACK,
		LEFT,
		TOP,
		BOTTOM
	};

	virtual bool Initialize();
	virtual bool Terminate();
	virtual bool Process();
	virtual bool Render();

	const mymath::AABB& GetAABB() const { return _aabb; }

	bool UpdateAABB();

	size_t GetVertexCount() const { return _cube_vertex.size(); }

protected:
	int _cube_handle;
	std::vector<VERTEX3D> _cube_vertex;
	std::vector<unsigned short> _cube_index;

	// 頂点用の固定情報
	VECTOR _cube_normal_front;
	VECTOR _cube_normal_right;
	VECTOR _cube_normal_back;
	VECTOR _cube_normal_left;
	VECTOR _cube_normal_top;
	VECTOR _cube_normal_bottom;
	std::array<VECTOR, 6> _cube_normal_list;
	// 基本の8頂点の座標
	VECTOR _cube_pos_0;
	VECTOR _cube_pos_1;
	VECTOR _cube_pos_2;
	VECTOR _cube_pos_3;
	VECTOR _cube_pos_4;
	VECTOR _cube_pos_5;
	VECTOR _cube_pos_6;
	VECTOR _cube_pos_7;
	std::array<VECTOR, 4> _cube_pos_front;
	std::array<VECTOR, 4> _cube_pos_right;
	std::array<VECTOR, 4> _cube_pos_back;
	std::array<VECTOR, 4> _cube_pos_left;
	std::array<VECTOR, 4> _cube_pos_top;
	std::array<VECTOR, 4> _cube_pos_bottom;

	std::vector<std::array<VECTOR, 4>> _cube_pos_list;

	VECTOR _cube_pos;

	mymath::AABB _aabb;

};

