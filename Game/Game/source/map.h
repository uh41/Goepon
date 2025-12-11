#pragma once
#include "objectbase.h"
#include "appframe.h"
#include "camera.h"
#include <map>

class Map : public ObjectBase
{
	typedef ObjectBase base;
public:
	virtual bool Initialize();
	virtual bool Terminate();
	virtual bool Process();
	virtual bool Render();

	auto& GetHandleMap() { return _handle_map; }
	auto GetHandleSkySphere() const { return _handle_sky_sphere; }
	auto GetFrameMapCollision() const { return _frame_map_collision; }
	auto GetHandleShadowMap() const { return _handle_shadow_map; }

	void SetCamera(Camera* cam) override { _cam = cam; }

protected:
	Camera* _cam;
	// マップ用
	// しまって取り出すときにstd::mapで管理
	int _handle_map;// std::map<マップ名, ハンドル>
	int _handle_sky_sphere;
	int _frame_map_collision;

	// シャドウマップ用
	int _handle_shadow_map;

	// 地面のテクスチャ
	int _ground_handle;
	std::vector<VERTEX3D> _ground_vertex;
	std::vector<unsigned short> _ground_index;

	// 地面全体の中心を原点にする
	float _start_x;
	float _start_z;
	// 頂点用の固定情報
	VECTOR _ground_normal;
	//地面の正方形用のテーブル
	VECTOR _ground_pos_0;
	VECTOR _ground_pos_1;
	VECTOR _ground_pos_2;
	VECTOR _ground_pos_3;
	std::array<VECTOR, 4> _ground_pos_list;

};

