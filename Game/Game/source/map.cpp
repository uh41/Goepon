/*********************************************************************/
// * \file   map.cpp
// * \brief  マップクラス
// *
// * \author 鈴木裕稀
// * \date   2025/12/25
// * \作業内容: 新規作成 鈴木裕稀　2025/12/15
//				Jsonファイル読み込み　鈴木裕稀　2025/12/25
/*********************************************************************/

#include "map.h"
#include "appframe.h"

// 初期化
bool Map::Initialize()
{
	if (!base::Initialize()) { return false; }

	// マップ
	_iHandleSkySphere = MV1LoadModel("res/SkySphere/skysphere.mv1");

	constexpr int MAP_SELECT = 4;

	if (MAP_SELECT == 0)
	{
		// ダンジョン
		_iHandleMap = MV1LoadModel("res/map/SM_map.mv1");
		_iFrameMapCollision = MV1SearchFrame(_iHandleMap, "pPlane1");

		// コリジョン情報の生成
		MV1SetupCollInfo(_iHandleMap, _iFrameMapCollision, 16, 16, 16);
		MV1SetFrameVisible(_iHandleMap, _iFrameMapCollision, FALSE);
	}
	else if (MAP_SELECT == 1)
	{
		// フィールド
		_iHandleMap = MV1LoadModel("res/Ground/Ground.mv1");
		_iFrameMapCollision = MV1SearchFrame(_iHandleMap, "ground_navmesh");

		// コリジョン情報の生成
		MV1SetupCollInfo(_iHandleMap, _iFrameMapCollision, 16, 16, 16);
		MV1SetFrameVisible(_iHandleMap, _iFrameMapCollision, FALSE);
	}
	else if (MAP_SELECT == 2)
	{
		_sPath = "res/map/";
		_sJsonFile = "maptry.json";
		_sJsonObjectName = "stage";

		_iFile.open(_sPath + _sJsonFile);
		nlohmann::json json;

		_iFile >> json;

		nlohmann::json stage = json.at(_sJsonObjectName);
		for (auto& data : stage)
		{
			mymath::BLOCKPOS pos;
			data.at("objectName").get_to(pos.name);
			// UEは左手座標系/Zup →左手座標系/Yup に変換しつつ取得
			data.at("translate").at("x").get_to(pos.x);
			data.at("translate").at("z").get_to(pos.y);
			data.at("translate").at("y").get_to(pos.z);
			pos.z *= -1.0f;
			data.at("rotate").at("x").get_to(pos.rx);
			data.at("rotate").at("z").get_to(pos.ry);
			data.at("rotate").at("y").get_to(pos.rz);
			pos.rx = DEG2RAD(pos.rx);
			pos.ry = DEG2RAD(pos.ry);
			pos.rz = DEG2RAD(pos.rz);
			data.at("scale").at("x").get_to(pos.sx);
			data.at("scale").at("z").get_to(pos.sy);
			data.at("scale").at("y").get_to(pos.sz);

			// 名前のモデルがすでに読み込み済か？
			if (_mModelHandle.count(pos.name) == 0)
			{
				// まだ読み込まれていない。読み込みを行う
				std::string filename = _sPath + pos.name + ".mv1";
				_mModelHandle[pos.name] = MV1LoadModel(filename.c_str());
			}
			// 名前から使うモデルハンドル＆表示フレームを決める
			if (_mModelHandle.count(pos.name) > 0)
			{
				pos.modelHandle = _mModelHandle[pos.name];
				pos.drawFrame = MV1SearchFrame(pos.modelHandle, pos.name.c_str());
			}

			_iFrameMapCollision = MV1SearchFrame(_mModelHandle[pos.name], "pPlane1");

			// コリジョン情報の生成
			MV1SetupCollInfo(_mModelHandle[pos.name], _iFrameMapCollision, 16, 16, 16);
			MV1SetFrameVisible(_mModelHandle[pos.name], _iFrameMapCollision, FALSE);

			// データをコンテナに追加（モデル番号があれば）
			if (pos.modelHandle != -1)
			{
				_vBlockPos.push_back(pos);
			}
		}


	}
	else if (MAP_SELECT == 3)
	{
		// 地面を使うパターン（モデルは読み込まない）
		_iHandleMap = -1;
		_iFrameMapCollision = -1;

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
	else if (MAP_SELECT == 4)
	{
		// フィールド
		_iHandleMap = MV1LoadModel("res/Dungeon/Dungeon.mv1");
		_iFrameMapCollision = MV1SearchFrame(_iHandleMap, "dungeon_collision");

		// コリジョン情報の生成
		MV1SetupCollInfo(_iHandleMap, _iFrameMapCollision, 16, 16, 16);
		MV1SetFrameVisible(_iHandleMap, _iFrameMapCollision, FALSE);
	}

	// コリジョン情報の生成
	MV1SetupCollInfo(_iHandleMap, _iFrameMapCollision, 16, 16, 16);// コリジョン情報を構築する(16以上は当たり判定を行う際に調べる区画の数が少なくなり、処理が速くなる)
	// コリジョンのフレームを描画しない設定
	MV1SetFrameVisible(_iHandleMap, _iFrameMapCollision, FALSE);

	// シャドウマップの生成
	_iHandleShadowMap = MakeShadowMap(2048, 2048);

	// ライト初期設定
	_mainLight.SetDir(VGet(-1.0f, -1.0f, 0.5f));

	// 環境光（弱め）
	_mainLight.SetAmbient(VGet(0.05f, 0.03f, 0.02f), 1.0f);

	// 拡散光（オレンジ寄り）
	_mainLight.SetDiffuse(VGet(1.0f, 0.75f, 0.45f), 1.0f);

	// 鏡面反射（白寄りに少し）
	_mainLight.SetSpecular(VGet(0.8f, 0.8f, 0.8f), 1.0f);

	_mainLight.SetCastShadow(true);
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
	const int extent = 800; // シャドウマップの範囲
	_mainLight.ApplyShadowMap(_iHandleShadowMap, DxlibConverter::VecToDxLib(_cam->_vTarget), extent);

	// 2回まわして、path = 0; シャドウマップへの描画、path = 1; モデルの描画(シャドウ適用）	
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
	SetShadowMapLightDirection(_iHandleShadowMap, lightdir);

	// シャドウマップに描画する範囲を設定
	// カメラの注視点を中心にする
	float lenght = 800.f;
	DxlibConverter::SetShadowMapDrawArea(
		_iHandleShadowMap,
		vec3::VAdd(_cam->_vTarget, vec3::VGet(-lenght, -1.0f, -lenght)),
		vec3::VAdd(_cam->_vTarget, vec3::VGet(lenght, lenght, lenght))
	);
	// 2回まわして、path = 0: シャドウマップへの描画、path = 1: モデルの：描画
	for(int path = 0; path < 2; path++)
	{
		if(path == 0)
		{
			// シャドウマップへの描画準備
			ShadowMap_DrawSetup(_iHandleShadowMap);

			// シャドウキャスターを描画(スカイは通常描かない)
			if(_iHandleMap >= 0) MV1DrawModel(_iHandleMap);
			for(auto & block : _vBlockPos)
			{
				MV1SetPosition(block.modelHandle, VGet(block.x, block.y, block.z));
				MV1SetRotationXYZ(block.modelHandle, VGet(block.rx, block.ry, block.rz));
				MV1SetScale(block.modelHandle, VGet(block.sx, block.sy, block.sz));
				MV1DrawModel(block.modelHandle);
			}
		}
		else // path == 1
		{
			// シャドウマップへの描画終了
			ShadowMap_DrawEnd();

			// 描画に使用するシャドウマップを設定
			SetUseShadowMap(0, _iHandleShadowMap);

			// ライト設定をレンダラに適用（アンビエント・ライト種別等）
			_mainLight.ApplyRenderer();

			// 本描画パス：マップ・スカイスフィア・ブロックを描画（シャドウマップが適用される）
			if (_iHandleMap >= 0) MV1DrawModel(_iHandleMap);
			if (_iHandleSkySphere >= 0) MV1DrawModel(_iHandleSkySphere);
			for (auto& block : _vBlockPos)
			{
				MV1SetPosition(block.modelHandle, VGet(block.x, block.y, block.z));
				MV1SetRotationXYZ(block.modelHandle, VGet(block.rx, block.ry, block.rz));
				MV1SetScale(block.modelHandle, VGet(block.sx, block.sy, block.sz));
				MV1DrawModel(block.modelHandle);
			}
		}
	}
	// シャドウマップの解除
	SetUseShadowMap(0, -1);

	if (_ground_handle == -1)
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

	for(auto & block : _vBlockPos)
	{
		// ブロックモデルを描画
		MV1SetPosition(block.modelHandle, VGet(block.x, block.y, block.z));
		MV1SetRotationXYZ(block.modelHandle, VGet(block.rx, block.ry, block.rz));
		MV1SetScale(block.modelHandle, VGet(block.sx, block.sy, block.sz));
		MV1DrawModel(block.modelHandle);
	}

	return true;
}
