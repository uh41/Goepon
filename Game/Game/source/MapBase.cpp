#include "MapBase.h"

// 初期化
bool MapBase::Initialize()
{
	if(!base::Initialize()) { return false; }
	_mapName.clear();

	// SkySphereの読み込み
	_iHandleSkySphere = ResourceServer::MV1LoadModel("res/SkySphere/skySphere.mv1");

	// ?V???h?E?}?b?v?????
	_iHandleShadowMap = MakeShadowMap(8192, 8192);

	// ライトの方向と色味（暖かめ・控えめ）
	_mainLight.SetDir(VGet(0.5f, -1.0f, 0.2f)); // 斜め上からの暖かい光（方向は必要に応じ調整）

	// アンビエント（低め・暖色）
	_mainLight.SetAmbient(VGet(0.02f, 0.015f, 0.01f), 1.0f); // 暖かい弱めの環境光

	// 拡散光（暖色寄りだが強すぎない）
	_mainLight.SetDiffuse(VGet(0.98f, 0.74f, 0.48f), 1.0f); // 明るさを抑えつつ暖色を強調

	// 鏡面反射（控えめで柔らかく）
	_mainLight.SetSpecular(VGet(0.35f, 0.30f, 0.25f), 1.0f);

	_mainLight.SetCastShadow(true);
	return true;
}

// 終了
bool MapBase::Terminate()
{
	base::Terminate();

	// 外部影キャスターを先に解除（キャプチャしている参照を切る）
	_externalShadowCasters = nullptr;

	// スカイスフィアモデルの削除
	if (_iHandleSkySphere >= 0)
	{
		ResourceServer::MV1DeleteModel(_iHandleSkySphere);
		_iHandleSkySphere = -1;
	}

	// メインマップモデルの削除
	if (_iHandleMap >= 0)
	{
		ResourceServer::MV1DeleteModel(_iHandleMap);
		_iHandleMap = -1;
	}

	// シャドウマップの削除
	if (_iHandleShadowMap >= 0)
	{
		DeleteShadowMap(_iHandleShadowMap);
		_iHandleShadowMap = -1;
	}

	// 地面テクスチャハンドルの削除（※現状ここが抜けている）
	if (_ground_handle >= 0)
	{
		DeleteGraph(_ground_handle);
		_ground_handle = -1;
	}

	// 個別モデルハンドルの削除
	for (auto& pair : _mModelHandle)
	{
		if (pair.second >= 0)
		{
			ResourceServer::MV1DeleteModel(pair.second);
		}
	}
	_mModelHandle.clear();

	// ベクターの解放
	_ground_vertex.clear();
	std::vector<VERTEX3D>().swap(_ground_vertex);

	_ground_index.clear();
	std::vector<unsigned short>().swap(_ground_index);

	_vBlockPos.clear();
	at::vet<mymath::BLOCKPOS>().swap(_vBlockPos);

	// ファイルストリームのクローズ
	if (_iFile.is_open())
	{
		_iFile.close();
	}

	return true;
}

// 更新
bool MapBase::Process()
{
	base::Process();

	auto offset_index = 0;
	_ground_vertex.clear();
	_ground_index.clear();

	// ?n??|???S??????_?E?C???f?b?N?X??
	for(int i = 0; i < GROUND_Z * GROUND_X; i++)
	{
		// ?s????v?Z
		int z = i / StCas<int>(GROUND_X);// ?s
		int x = i % StCas<int>(GROUND_X);// ??

		// ?I?t?Z?b?g?v?Z
		auto offset_x = _start_x + StCas<float>(x) * GROUND_POLYGON_SIZE;
		auto offset_z = _start_z + StCas<float>(z) * GROUND_POLYGON_SIZE;

		// 4???_???
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

		// 2????O?p?`??C???f?b?N?X
		auto index = StCas<unsigned short>(i * 4);
		_ground_index.push_back(StCas<unsigned short>(index + 0));
		_ground_index.push_back(StCas<unsigned short>(index + 1));
		_ground_index.push_back(StCas<unsigned short>(index + 2));
		_ground_index.push_back(StCas<unsigned short>(index + 2));
		_ground_index.push_back(StCas<unsigned short>(index + 1));
		_ground_index.push_back(StCas<unsigned short>(index + 3));
	}

	// ?u???b?N???u?E??]?E?X?P?[?????
	for(auto& block : _vBlockPos)
	{
		// ???f???n???h???????????X?L?b?v
		if(block.modelHandle < 0)
		{
			continue;
		}
		// ??u?E??]?E?X?P?[?????
		MV1SetPosition(block.modelHandle, VGet(block.x, block.y, block.z));
		MV1SetRotationXYZ(block.modelHandle, VGet(block.rx, block.ry, block.rz));
		MV1SetScale(block.modelHandle, VGet(block.sx, block.sy, block.sz));

		// ?R???W???????? transform ??????X?V?i?d?v?j
		MV1RefreshCollInfo(block.modelHandle, -1);
	}

	// SkySphere?i?X?P?[???E??u?j???
	if(_iHandleSkySphere >= 0)
	{
		// ??F200?{?i?K?v??l??????j
		float kSkySphereScale = 200.0f;
		MV1SetScale(_iHandleSkySphere, VGet(kSkySphereScale, kSkySphereScale, kSkySphereScale));

		// ???_?????????[??????????A?J???????]??????i?????j
		if(_cam)
		{
			MV1SetPosition(_iHandleSkySphere, DxlibConverter::VecToDxLib(_cam->GetPos()));
		}

	}

	return true;
}

// 描画
bool MapBase::Render()
{
	base::Render();

	// 3D基本設定
	SetUseZBuffer3D(true);
	SetWriteZBuffer3D(true);
	SetUseBackCulling(true);

	// ライト方向を固定
	VECTOR lightdir = VGet(1.0f, -10.0f, 0.5f); // より自然な斜め上からの光

#if 1 // 平行ライト
	SetGlobalAmbientLight(GetColorF(0.03f, 0.025f, 0.02f, 1.0f)); // 少し明るく
	ChangeLightTypeDir(lightdir);
#endif
#if 0 // ポイントライト
	SetGlobalAmbientLight(GetColorF(0.f, 0.f, 0.f, 0.f));
	ChangeLightTypePoint(VAdd(_pos, VGet(0.50f, 0)), 1000.f, 0.f, 0.005f, 0.f);
#endif

	// シャドウマップを投射するライトの方向をセット
	SetShadowMapLightDirection(_iHandleShadowMap, lightdir);

	// シャドウマップの範囲を大幅に縮小（解像度向上のため）
	const float shadowRange = 800.0f; // 5000 -> 800に変更
	const float shadowHeight = 400.0f; // 高さ方向の範囲を制限
	VECTOR shadowCenter = VGet(0.0f, 0.0f, 0.0f); // シャドウマップの中心位置（固定）

	// シャドウマップの描画範囲を設定（Near/Farを適切に）
	VECTOR shadowMin = VAdd(shadowCenter, VGet(-shadowRange, -shadowHeight * 0.5f, -shadowRange));
	VECTOR shadowMax = VAdd(shadowCenter, VGet(shadowRange, shadowHeight * 0.5f, shadowRange));

	// DxLibのVECTORをvec3::Vec3に変換してから渡す
	SetShadowMapDrawArea(_iHandleShadowMap, shadowMin, shadowMax);

	// ライト設定をシャドウマップ用に設定
	const int extent = 5000; // 5000 -> 800に変更
	_mainLight.ApplyShadowMap(_iHandleShadowMap, shadowCenter, extent);

	// 2回まわして、path = 0: シャドウマップへの描画、path = 1: モデルの描画
	for(int path = 0; path < 2; path++)
	{
		if(path == 0)
		{
			// シャドウマップへの描画開始
			ShadowMap_DrawSetup(_iHandleShadowMap);

			// 地面もシャドウキャスターに追加
			if(_ground_handle != -1)
			{
				//auto vertex_num = StCas<int>(_ground_vertex.size());
				//auto index_num = StCas<int>(_ground_index.size());

				//// シャドウマップへの描画は、頂点数とインデックス数が3以上必要
				//if(vertex_num >= 3 && index_num >= 3)
				//{
				//	auto polygon_num = index_num / 3;
				//	DrawPolygonIndexed3D
				//	(
				//		_ground_vertex.data(),
				//		vertex_num,
				//		_ground_index.data(),
				//		polygon_num,
				//		_ground_handle,
				//		FALSE
				//	);
				//}

				//// シャドウキャスター描画
				//if(_iHandleMap >= 0) MV1DrawModel(_iHandleMap);
				//for(auto& block : _vBlockPos)
				//{
				//	MV1SetPosition(block.modelHandle, VGet(block.x, block.y, block.z));
				//	MV1SetRotationXYZ(block.modelHandle, VGet(block.rx, block.ry, block.rz));
				//	MV1SetScale(block.modelHandle, VGet(block.sx, block.sy, block.sz));
				//	MV1DrawModel(block.modelHandle);
				//}

				if(_externalShadowCasters)
				{
					_externalShadowCasters();
				}
			}
		}
		else // path == 1
		{
			// シャドウマップへの描画終了
			ShadowMap_DrawEnd();

			// 描画に使用するシャドウマップを設定
			SetUseShadowMap(0, _iHandleShadowMap);

			// ライト設定を頂点色に適用(アンビエント・ライト反映等)
			_mainLight.ApplyRenderer();

			// 本描画パス：マップ・スカイスフィア・ブロック描画(シャドウマップが適用される)
			if(_iHandleMap >= 0) MV1DrawModel(_iHandleMap);
			if(_iHandleSkySphere >= 0) MV1DrawModel(_iHandleSkySphere);
			for(auto& block : _vBlockPos)
			{
				MV1SetPosition(block.modelHandle, VGet(block.x, block.y, block.z));
				MV1SetRotationXYZ(block.modelHandle, VGet(block.rx, block.ry, block.rz));
				MV1SetScale(block.modelHandle, VGet(block.sx, block.sy, block.sz));
				MV1DrawModel(block.modelHandle);
			}

			// 地面描画
			if(_ground_handle != -1)
			{
				auto vertex_num = StCas<int>(_ground_vertex.size());
				auto index_num = StCas<int>(_ground_index.size());

				if(vertex_num >= 3 && index_num >= 3)
				{
					auto polygon_num = index_num / 3;
					DrawPolygonIndexed3D(_ground_vertex.data(), vertex_num, _ground_index.data(), polygon_num, _ground_handle, FALSE);
				}
			}
		}
	}
	// シャドウマップの解除
	SetUseShadowMap(0, -1);
	return true;
}

// モデルの読み込み
bool MapBase::LoadModel(std::string fileName, std::string attachFrameName)
{
	// モデルの読み込み
	_iHandleMap = ResourceServer::MV1LoadModel(fileName.c_str());
	if(_handle < 0)
	{
		return false;
	}

	// アタッチフレーム名が指定されていれば、アタッチフレームのインデックスを取得
	_iFrameMapCollision = -1;
	if (!attachFrameName.empty())
	{
		_iFrameMapCollision = MV1SearchFrame(_iHandleMap, attachFrameName.c_str());
		if (_iFrameMapCollision >= 0)
		{
			MV1SetFrameVisible(_iHandleMap, _iFrameMapCollision, FALSE);
		}
	}
	return true;
}
