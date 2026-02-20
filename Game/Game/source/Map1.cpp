#include "Map1.h"
#include "applicationglobal.h"

// 初期化
bool Map1::Initialize()
{
	// 基底クラス初期化
	if(!base::Initialize()) { return false; }

	// スカイスフィア読み込み
	_iHandleSkySphere = MV1LoadModel("res/SkySphere/skysphere.mv1");

	const ApplicationGlobal::MapData* map = gGlobal.GetMapData("Map1");
	if(map)
	{
		_mModelHandle = map->modelHandle;
		_vBlockPos = map->blockPos;

		// コリジョン情報の更新
		for(auto& block : _vBlockPos)
		{
			if(block.modelHandle >= 0)
			{
				MV1SetPosition(block.modelHandle, VGet(block.x, block.y, block.z));
				MV1SetRotationXYZ(block.modelHandle, VGet(block.rx, block.ry, block.rz));
				MV1SetScale(block.modelHandle, VGet(block.sx, block.sy, block.sz));
				MV1RefreshCollInfo(block.modelHandle, -1);
			}
		}
	}
	_ground_handle = LoadGraph(img::BG_stone);
	// 以降の初期化（省略せず元の処理を入れてください）
	_half_polygon_size = GROUND_POLYGON_SIZE * 0.5f;
	_start_x = -_half_polygon_size * StCas<float>(GROUND_X);
	_start_z = -_half_polygon_size * StCas<float>(GROUND_Z);
	_ground_normal = VGet(0.0f, 1.0f, 0.0f);
	_diffuse = GetColorU8(255, 255, 255, 255);
	_specular = GetColorU8(0, 0, 0, 0);
	_ground_pos_0 = VGet(0.0f, -20.0f, 0.0f);
	_ground_pos_1 = VGet(0.0f, -20.0f, GROUND_POLYGON_SIZE);
	_ground_pos_2 = VGet(GROUND_POLYGON_SIZE, -20.0f, 0.0f);
	_ground_pos_3 = VGet(GROUND_POLYGON_SIZE, -20.0f, GROUND_POLYGON_SIZE);
	_ground_pos_list = { _ground_pos_0, _ground_pos_1, _ground_pos_2, _ground_pos_3 };
	_u_list = { 0.0f, 0.0f, 1.0f, 1.0f };
	_v_list = { 0.0f, 1.0f, 0.0f, 1.0f };

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
bool Map1::Terminate()
{
	base::Terminate();
	return true;
}

// 更新処理
bool Map1::Process()
{
	base::Process();

	auto offset_index = 0;
	_ground_vertex.clear();
	_ground_index .clear();
	//_ground_vertex.reserve(static_cast<int>(GROUND_Z * GROUND_X) * 4);
	//_ground_index.reserve(static_cast<int>(GROUND_Z * GROUND_X) * 6);

	// 地面ポリゴンの頂点・インデックス作成
	for(int i = 0; i < GROUND_Z * GROUND_X; i++)
	{
		// 行と列の計算
		int z = i / StCas<int>(GROUND_X);// 行
		int x = i % StCas<int>(GROUND_X);// 列

		// オフセット計算
		auto offset_x = _start_x + StCas<float>(x) * GROUND_POLYGON_SIZE;
		auto offset_z = _start_z + StCas<float>(z) * GROUND_POLYGON_SIZE;

		// 4頂点の作成
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
		auto index = StCas<unsigned short>(i * 4);
		_ground_index.push_back(StCas<unsigned short>(index + 0));
		_ground_index.push_back(StCas<unsigned short>(index + 1));
		_ground_index.push_back(StCas<unsigned short>(index + 2));
		_ground_index.push_back(StCas<unsigned short>(index + 2));
		_ground_index.push_back(StCas<unsigned short>(index + 1));
		_ground_index.push_back(StCas<unsigned short>(index + 3));
	}

	// ブロックの位置・回転・スケール設定
	for(auto& block : _vBlockPos)
	{
		// モデルハンドルが無効ならスキップ
		if(block.modelHandle < 0)
		{
			continue;
		}
		// 位置・回転・スケール設定
		MV1SetPosition(block.modelHandle, VGet(block.x, block.y, block.z));
		MV1SetRotationXYZ(block.modelHandle, VGet(block.rx, block.ry, block.rz));
		MV1SetScale(block.modelHandle, VGet(block.sx, block.sy, block.sz));

		// コリジョン情報を transform 後の状態に更新（重要）
		MV1RefreshCollInfo(block.modelHandle, -1);
	}

	// SkySphere（スケール・位置）設定
	if(_iHandleSkySphere >= 0)
	{
		// 例：200倍（必要な値に調整）
		float kSkySphereScale = 200.0f;
		MV1SetScale(_iHandleSkySphere, VGet(kSkySphereScale, kSkySphereScale, kSkySphereScale));

		// 原点固定だと移動で端が見えるので、カメラに追従させる（推奨）
		if(_cam)
		{
			MV1SetPosition(_iHandleSkySphere, DxlibConverter::VecToDxLib(_cam->_vPos));
		}

		// 変換後の当たり判定更新が不要なら、この行はいりません
		// MV1RefreshCollInfo(_iHandleSkySphere, -1);
	}

	return true;
}

// 描画処理
bool Map1::Render()
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
			for(auto& block : _vBlockPos)
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
			if(_iHandleMap >= 0) MV1DrawModel(_iHandleMap);
			if(_iHandleSkySphere >= 0) MV1DrawModel(_iHandleSkySphere);
			for(auto& block : _vBlockPos)
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

	if(_ground_handle == -1)
	{
		return false;
	}

	// 地面を描画
	auto vertex_num = StCas<int>(_ground_vertex.size());
	auto index_num = StCas<int>(_ground_index.size());

	// ポリゴンが1つもなければ描画しない
	if(3 > vertex_num || 3 > index_num)
	{
		return false;
	}
	auto polygon_num = index_num / 3;
	DrawPolygonIndexed3D(_ground_vertex.data(), vertex_num, _ground_index.data(), polygon_num, _ground_handle, FALSE);

	for(auto& block : _vBlockPos)
	{
		// ブロックモデルを描画
		MV1SetPosition(block.modelHandle, VGet(block.x, block.y, block.z));
		MV1SetRotationXYZ(block.modelHandle, VGet(block.rx, block.ry, block.rz));
		MV1SetScale(block.modelHandle, VGet(block.sx, block.sy, block.sz));
		MV1DrawModel(block.modelHandle);
	}
	return true;
}