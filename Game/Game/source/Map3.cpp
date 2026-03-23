#include "Map3.h"
#include "applicationglobal.h"

bool Map3::Initialize()
{
	if (!base::Initialize()) { return false; }

	// SkySphereの読み込み
	_iHandleSkySphere = ResourceServer::MV1LoadModel("res/SkySphere/skySphere.mv1");

	const ApplicationGlobal::MapData* map = gGlobal.GetMapData("Map3");
	if(map)
	{
		_mModelHandle = map->modelHandle;
		_vBlockPos = map->blockPos;

		// ?R???W????????X?V
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
	// ??~????????i????????????????????????????j
	_half_polygon_size = GROUND_POLYGON_SIZE * 0.5f;
	_start_x = -_half_polygon_size * StCas<float>(GROUND_X);
	_start_z = -_half_polygon_size * StCas<float>(GROUND_Z);
	_ground_normal = VGet(0.0f, 1.0f, 0.0f);
	_diffuse = GetColorU8(255, 255, 255, 255);
	_specular = GetColorU8(0, 0, 0, 0);
	_ground_pos_0 = VGet(0.0f, -100.0f, 0.0f);
	_ground_pos_1 = VGet(0.0f, -100.0f, GROUND_POLYGON_SIZE);
	_ground_pos_2 = VGet(GROUND_POLYGON_SIZE, -100.0f, 0.0f);
	_ground_pos_3 = VGet(GROUND_POLYGON_SIZE, -100.0f, GROUND_POLYGON_SIZE);
	_ground_pos_list = { _ground_pos_0, _ground_pos_1, _ground_pos_2, _ground_pos_3 };
	_u_list = { 0.0f, 0.0f, 1.0f, 1.0f };
	_v_list = { 0.0f, 1.0f, 0.0f, 1.0f };

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

// ?I??
bool Map3::Terminate()
{
	// スカイスフィアモデルの削除
	if(_iHandleSkySphere >= 0)
	{
		ResourceServer::MV1DeleteModel(_iHandleSkySphere);
		_iHandleSkySphere = -1;
	}

	// メインマップモデルの削除
	if(_iHandleMap >= 0)
	{
		ResourceServer::MV1DeleteModel(_iHandleMap);
		_iHandleMap = -1;
	}

	// シャドウマップの削除（そのまま）
	if(_iHandleShadowMap >= 0)
	{
		DeleteShadowMap(_iHandleShadowMap);
		_iHandleShadowMap = -1;
	}

	// 個別モデルハンドルの削除
	for(auto& pair : _mModelHandle)
	{
		if(pair.second >= 0)
		{
			ResourceServer::MV1DeleteModel(pair.second);
		}
	}
	_mModelHandle.clear();

	// ベクターのクリア（容量ごと解放）
	_ground_vertex.clear();
	std::vector<VERTEX3D>().swap(_ground_vertex);         // メモリを確実に解放

	_ground_index.clear();
	std::vector<unsigned short>().swap(_ground_index);    // メモリを確実に解放

	_vBlockPos.clear();
	at::vet<mymath::BLOCKPOS>().swap(_vBlockPos);        // at::vet の場合も再構築で解放促進

	// ファイルストリームのクローズ
	if(_iFile.is_open())
	{
		_iFile.close();
	}

	// 基底クラスの終了処理
	base::Terminate();
	return true;
}

// ?X?V????
bool Map3::Process()
{
	base::Process();

	auto offset_index = 0;
	_ground_vertex.clear();
	_ground_index.clear();
	//_ground_vertex.reserve(static_cast<int>(GROUND_Z * GROUND_X) * 4);
	//_ground_index.reserve(static_cast<int>(GROUND_Z * GROUND_X) * 6);

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

		// ?????????????X?V???s?v???A????s?????????
		// MV1RefreshCollInfo(_iHandleSkySphere, -1);
	}

	return true;
}

// ?`????
bool Map3::Render()
{
	base::Render();


	return true;
}