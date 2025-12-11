#include "map.h"

// 初期化
bool Map::Initialize()
{
	if(!base::Initialize()) { return false; }

	// マップ
	_handle_sky_sphere = MV1LoadModel("res/SkySphere/skysphere.mv1");

	constexpr int MAP_SELECT = 0;

	if(MAP_SELECT == 0)
	{
		// ダンジョン
		_handle_map = MV1LoadModel("res/Dungeon/Dungeon.mv1");
		_frame_map_collision = MV1SearchFrame(_handle_map, "dungeon_collision");

		// コリジョン情報の生成
		MV1SetupCollInfo(_handle_map, _frame_map_collision, 16, 16, 16);
		MV1SetFrameVisible(_handle_map, _frame_map_collision, FALSE);
	}
	else if(MAP_SELECT == 1)
	{
		// フィールド
		_handle_map = MV1LoadModel("res/Ground/Ground.mv1");
		_frame_map_collision = MV1SearchFrame(_handle_map, "ground_navmesh");

		// コリジョン情報の生成
		MV1SetupCollInfo(_handle_map, _frame_map_collision, 16, 16, 16);
		MV1SetFrameVisible(_handle_map, _frame_map_collision, FALSE);
	}
	else if(MAP_SELECT == 2)
	{
		// 地面を使うパターン（モデルは読み込まない）
		_handle_map = -1;
		_frame_map_collision = -1;

		_ground_handle = LoadGraph("res/Texture/Groundplants1_D.jpg");
		// 以降の初期化（省略せず元の処理を入れてください）
		_half_polygon_size = GROUND_POLYGON_SIZE * 0.5f;
		_start_x = -_half_polygon_size * static_cast<float>(GROUND_X);
		_start_z = -_half_polygon_size * static_cast<float>(GROUND_Z);
		_ground_normal = VGet(0.0f, 1.0f, 0.0f);
		_diffuse = GetColorU8(255, 255, 255, 255);
		_specular = GetColorU8(0, 0, 0, 0);
		_ground_pos_0 = VGet(0.0f, 0.0f, 0.0f);
		_ground_pos_1 = VGet(0.0f, 0.0f, GROUND_POLYGON_SIZE);
		_ground_pos_2 = VGet(GROUND_POLYGON_SIZE, 0.0f, 0.0f);
		_ground_pos_3 = VGet(GROUND_POLYGON_SIZE, 0.0f, GROUND_POLYGON_SIZE);
		_ground_pos_list = { _ground_pos_0, _ground_pos_1, _ground_pos_2, _ground_pos_3 };
		_u_list = { 0.0f, 0.0f, 1.0f, 1.0f };
		_v_list = { 0.0f, 1.0f, 0.0f, 1.0f };
	}
	// コリジョン情報の生成
	MV1SetupCollInfo(_handle_map, _frame_map_collision, 16, 16, 16);// コリジョン情報を構築する(16以上は当たり判定を行う際に調べる区画の数が少なくなり、処理が速くなる)
	// コリジョンのフレームを描画しない設定
	MV1SetFrameVisible(_handle_map, _frame_map_collision, FALSE);

	// シャドウマップの生成
	_handle_shadow_map = MakeShadowMap(2048, 2048);

	return true;
}

// 終了
bool Map::Terminate()
{

	base::Terminate();
	return true;
}

// 計算処理
bool Map::Process()
{
	base::Process();

	auto offset_index = 0;
	_ground_vertex.clear();
	_ground_index.clear();
	//_ground_vertex.reserve(static_cast<int>(GROUND_Z * GROUND_X) * 4);
	//_ground_index.reserve(static_cast<int>(GROUND_Z * GROUND_X) * 6);

	for(int i= 0; i < GROUND_Z * GROUND_X; i++)
	{
		int z = i / static_cast<int>(GROUND_X);// 行
		int x = i % static_cast<int>(GROUND_X);// 列

		auto offset_x = _start_x + static_cast<float>(x) * GROUND_POLYGON_SIZE;
		auto offset_z = _start_z + static_cast<float>(z) * GROUND_POLYGON_SIZE;

		for(int j = 0; j < 4; j++)
		{
			VERTEX3D vertex;
			VECTOR base = _ground_pos_list[j];

			base.x += offset_x;
			base.z += offset_z;

			vertex.pos = base;
			vertex.norm = _ground_normal;
			vertex.dif = _diffuse;
			vertex.spc = _specular;
			vertex.u = _u_list[j];
			vertex.v = _v_list[j];

			_ground_vertex.push_back(vertex);
		}

		// 2枚の三角形のインデックス
		auto index = static_cast<unsigned short>(i * 4);
		_ground_index.push_back(static_cast<unsigned short>(index + 0));
		_ground_index.push_back(static_cast<unsigned short>(index + 1));
		_ground_index.push_back(static_cast<unsigned short>(index + 2));
		_ground_index.push_back(static_cast<unsigned short>(index + 2));
		_ground_index.push_back(static_cast<unsigned short>(index + 1));
		_ground_index.push_back(static_cast<unsigned short>(index + 3));
	}


	return true;
}

// 描画処理
bool Map::Render()
{
	base::Render();

	// 3D基本設定
	SetUseZBuffer3D(TRUE);
	SetWriteZBuffer3D(TRUE);
	SetUseBackCulling(TRUE);

	// ライト設定
	SetUseLighting(TRUE);

	VECTOR lightdir = VGet(-1.0f, -1.0f, 0.5f);
#if 1 // 平行ライト
	SetGlobalAmbientLight(GetColorF(0.f, 0.f, 0.f, 0.f));
	ChangeLightTypeDir(lightdir);
#endif
#if 0 // ポイントライト
	SetGlobalAmbientLight(GetColorF(0.f, 0.f, 0.f, 0.f));
	ChangeLightTypePoint(VAdd(_pos, VGet(0.50f, 0)), 1000.f, 0.f, 0.005f, 0.f);
#endif

	// シャドウマップが想定するライトの方向もセット
	SetShadowMapLightDirection(_handle_shadow_map, lightdir);

	// シャドウマップに描画する範囲を設定
	// カメラの注視点を中心にする
	float lenght = 800.f;
	SetShadowMapDrawArea(_handle_shadow_map, VAdd(_cam->_v_target, VGet(-lenght, -1.0f, -lenght)), VAdd(_cam->_v_target, VGet(lenght, lenght, lenght)));

	// 2回まわして、path = 0: シャドウマップへの描画、path = 1: モデルの：描画
	for(int path = 0; path < 2; path++)
	{
		if(path == 0)
		{
			// シャドウマップへの描画の準備
			ShadowMap_DrawSetup(_handle_shadow_map);
		}
		else if(path == 1)
		{
			// シャドウマップへの描画終了
			ShadowMap_DrawEnd();
			// 描画に使用するシャドウマップを設定
			SetUseShadowMap(0, _handle_shadow_map);
		}

		// マップモデルを描画
		{
			MV1DrawModel(_handle_map);
			MV1DrawModel(_handle_sky_sphere);
		}
	}
	SetUseShadowMap(0, -1); // シャドウマップの解除

	if(_ground_handle == -1)
	{
		return false;
	}

	// 地面を描画
	auto vertex_num = static_cast<int>(_ground_vertex.size());
	auto index_num = static_cast<int>(_ground_index.size());

	// ポリゴンが1つもなければ描画しない
	if(3 > vertex_num || 3 > index_num)
	{
		return false;
	}
	auto polygon_num = index_num / 3;
	DrawPolygonIndexed3D(_ground_vertex.data(), vertex_num, _ground_index.data(), polygon_num, _ground_handle, FALSE);

	return true;
}
