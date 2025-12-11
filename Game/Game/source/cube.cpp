#include "cube.h"

// 初期化
bool Cube::Initialize()
{
	if(!base::Initialize()) { return false; }
	
	_cube_handle = LoadGraph("res/Texture/kime-yoko.jpg");
	// 地面全体の中心を原点にする
	_half_polygon_size = GROUND_POLYGON_SIZE * 0.5f;
	// 頂点用の固定情報
	_cube_normal_front = VGet(0.0f, 0.0f, -1.0f);
	_cube_normal_right = VGet(1.0f, 0.0f, 0.0f);
	_cube_normal_back = VGet(0.0f, 0.0f, 1.0f);
	_cube_normal_left = VGet(-1.0f, 0.0f, 0.0f);
	_cube_normal_top = VGet(0.0f, 1.0f, 0.0f);
	_cube_normal_bottom = VGet(0.0f, -1.0f, 0.0f);
	_cube_normal_list =
	{
		_cube_normal_front,
		_cube_normal_right,
		_cube_normal_back,
		_cube_normal_left,
		_cube_normal_top,
		_cube_normal_bottom
	};
	_diffuse = GetColorU8(255, 255, 255, 255);
	_specular = GetColorU8(0, 0, 0, 0);
	// 基本の8頂点の座標
	_cube_pos_0 = VGet(-_half_polygon_size,  _half_polygon_size, -_half_polygon_size);
	_cube_pos_1 = VGet(-_half_polygon_size,  _half_polygon_size,  _half_polygon_size);
	_cube_pos_2 = VGet( _half_polygon_size,  _half_polygon_size, -_half_polygon_size);
	_cube_pos_3 = VGet( _half_polygon_size,  _half_polygon_size,  _half_polygon_size);
	_cube_pos_4 = VGet(-_half_polygon_size, -_half_polygon_size, -_half_polygon_size);
	_cube_pos_5 = VGet(-_half_polygon_size, -_half_polygon_size,  _half_polygon_size);
	_cube_pos_6 = VGet( _half_polygon_size, -_half_polygon_size, -_half_polygon_size);
	_cube_pos_7 = VGet( _half_polygon_size, -_half_polygon_size,  _half_polygon_size);
	_cube_pos_front =
	{
		_cube_pos_4,
		_cube_pos_0,
		_cube_pos_6,
		_cube_pos_2
	};
	_cube_pos_right =
	{
		_cube_pos_6,
		_cube_pos_2,
		_cube_pos_7,
		_cube_pos_3
	};
	_cube_pos_back =
	{
		_cube_pos_7,
		_cube_pos_3,
		_cube_pos_5,
		_cube_pos_1
	};
	_cube_pos_left =
	{
		_cube_pos_5,
		_cube_pos_1,
		_cube_pos_4,
		_cube_pos_0
	};
	_cube_pos_top =
	{
		_cube_pos_0,
		_cube_pos_1,
		_cube_pos_2,
		_cube_pos_3
	};
	_cube_pos_bottom =
	{
		_cube_pos_4,
		_cube_pos_5,
		_cube_pos_6,
		_cube_pos_7
	};
	//処理: リストのメモリ領域に、直接データを生成します。無駄なコピーや移動の手間が省けるため、処理速度が速くなる傾向があります
	_cube_pos_list.emplace_back(_cube_pos_front);
	_cube_pos_list.emplace_back(_cube_pos_right);
	_cube_pos_list.emplace_back(_cube_pos_back);
	_cube_pos_list.emplace_back(_cube_pos_left);
	_cube_pos_list.emplace_back(_cube_pos_top);
	_cube_pos_list.emplace_back(_cube_pos_bottom);
	// uvのテーブル
	_u_list = { 0.0f, 0.0f, 1.0f, 1.0f };
	_v_list = { 0.0f, 1.0f, 0.0f, 1.0f };

	_cube_pos = VGet(300.0f, 0.0f, 0.0f);

	UpdateAABB();
	return true;
}

// AABB更新
bool Cube::UpdateAABB()
{
	std::array<VECTOR, 8> pt =
	{
		_cube_pos_0, _cube_pos_1, _cube_pos_2, _cube_pos_3,
		_cube_pos_4, _cube_pos_5, _cube_pos_6, _cube_pos_7
	};

	VECTOR min = VAdd(pt[0], _cube_pos);
	VECTOR max = min;
	for(auto& p : pt)
	{
		VECTOR wp = VAdd(p, _cube_pos);
		if(wp.x < min.x)
		{
			min.x = wp.x;
		}
		if(wp.y < min.y)
		{
			min.y = wp.y;
		}
		if(wp.z < min.z)
		{
			min.z = wp.z;
		}

		if(wp.x > max.x)
		{
			max.x = wp.x;
		}
		if(wp.y > max.y)
		{
			max.y = wp.y;
		}
		if(wp.z > max.z)
		{
			max.z = wp.z;
		}
	}
	_aabb.min = min;
	_aabb.max = max;
	return true;
}

// 終了
bool Cube::Terminate()
{
	base::Terminate();
	return true;
}

// 計算処理
bool Cube::Process()
{
	base::Process();
	// 頂点、インデックス情報のクリア
	_cube_vertex.clear();
	_cube_index.clear();

	// cubeの6面
	for(int i = 0; i < 6 * 4; i++)
	{
		auto face = i / 4;
		auto j = i % 4;

		VERTEX3D vertex;
		auto face_vertices = _cube_pos_list[face];

		VECTOR base = face_vertices[j];

		vertex.pos = VAdd(base,_cube_pos);
		vertex.norm = _cube_normal_list[face];
		vertex.dif = _diffuse;
		vertex.spc = _specular;
		vertex.u = _u_list[j];
		vertex.v = _v_list[j];

		this->_cube_vertex.emplace_back(vertex);

		if(j == 3)
		{
			int baseindex = static_cast<int>(_cube_vertex.size()) - 4;
			// 2枚の三角形のインデックス
			_cube_index.push_back(static_cast<unsigned short>(baseindex + 0));
			_cube_index.push_back(static_cast<unsigned short>(baseindex + 1));
			_cube_index.push_back(static_cast<unsigned short>(baseindex + 2));

			_cube_index.push_back(static_cast<unsigned short>(baseindex + 2));
			_cube_index.push_back(static_cast<unsigned short>(baseindex + 1));
			_cube_index.push_back(static_cast<unsigned short>(baseindex + 3));

		}
	}

	return true;
}

// 描画処理
bool Cube::Render()
{
	base::Render();

	if(_cube_handle == -1)
	{
		return false;
	}

	// 描画
	auto vertex_num = static_cast<int>(_cube_vertex.size());
	auto index_num = static_cast<int>(_cube_index.size());

	// ポリゴンが1つもない場合は描画しない
	if(vertex_num == 0 || index_num == 0)
	{
		return false;
	}

	auto polygon_num = index_num / 3;
	DrawPolygonIndexed3D(_cube_vertex.data(), vertex_num, _cube_index.data(), polygon_num, _cube_handle, FALSE);

	return true;
}
