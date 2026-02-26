#include "Map1.h"
#include "applicationglobal.h"

// ������
bool Map1::Initialize()
{
	// ���N���X������
	if(!base::Initialize()) { return false; }

	// �X�J�C�X�t�B�A�ǂݍ���
	_iHandleSkySphere = MV1LoadModel("res/SkySphere/skysphere.mv1");

	const ApplicationGlobal::MapData* map = gGlobal.GetMapData("Map1");
	if(map)
	{
		_mModelHandle = map->modelHandle;
		_vBlockPos = map->blockPos;

		// �R���W�������̍X�V
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
	// �ȍ~�̏������i�ȗ��������̏��������Ă��������j
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

	// �V���h�E�}�b�v�̐���
	_iHandleShadowMap = MakeShadowMap(2048, 2048);

	// ���C�g�����ݒ�
	_mainLight.SetDir(VGet(-1.0f, -1.0f, 0.5f));

	// �����i��߁j
	_mainLight.SetAmbient(VGet(0.05f, 0.03f, 0.02f), 1.0f);

	// �g�U���i�I�����W���j
	_mainLight.SetDiffuse(VGet(1.0f, 0.75f, 0.45f), 1.0f);

	// ���ʔ��ˁi�����ɏ����j
	_mainLight.SetSpecular(VGet(0.8f, 0.8f, 0.8f), 1.0f);

	_mainLight.SetCastShadow(true);

	return true;
}

// �I��
bool Map1::Terminate()
{
	// スカイスフィアモデルの削除
	if(_iHandleSkySphere >= 0)
	{
		MV1DeleteModel(_iHandleSkySphere);
		_iHandleSkySphere = -1;
	}

	// シャドウマップの削除
	if(_iHandleShadowMap >= 0)
	{
		DeleteShadowMap(_iHandleShadowMap);
		_iHandleShadowMap = -1;
	}

	// 地面テクスチャハンドルの削除
	if(_ground_handle >= 0)
	{
		DeleteGraph(_ground_handle);
		_ground_handle = -1;
	}

	// ベクターのクリア（メモリ解放）
	_ground_vertex.clear();
	_ground_index.clear();
	_vBlockPos.clear();

	// 基底クラスの終了処理
	base::Terminate();
	return true;
}

// �X�V����
bool Map1::Process()
{
	base::Process();

	auto offset_index = 0;
	_ground_vertex.clear();
	_ground_index .clear();
	//_ground_vertex.reserve(static_cast<int>(GROUND_Z * GROUND_X) * 4);
	//_ground_index.reserve(static_cast<int>(GROUND_Z * GROUND_X) * 6);

	// �n�ʃ|���S���̒��_�E�C���f�b�N�X�쐬
	for(int i = 0; i < GROUND_Z * GROUND_X; i++)
	{
		// �s�Ɨ�̌v�Z
		int z = i / StCas<int>(GROUND_X);// �s
		int x = i % StCas<int>(GROUND_X);// ��

		// �I�t�Z�b�g�v�Z
		auto offset_x = _start_x + StCas<float>(x) * GROUND_POLYGON_SIZE;
		auto offset_z = _start_z + StCas<float>(z) * GROUND_POLYGON_SIZE;

		// 4���_�̍쐬
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

		// 2���̎O�p�`�̃C���f�b�N�X
		auto index = StCas<unsigned short>(i * 4);
		_ground_index.push_back(StCas<unsigned short>(index + 0));
		_ground_index.push_back(StCas<unsigned short>(index + 1));
		_ground_index.push_back(StCas<unsigned short>(index + 2));
		_ground_index.push_back(StCas<unsigned short>(index + 2));
		_ground_index.push_back(StCas<unsigned short>(index + 1));
		_ground_index.push_back(StCas<unsigned short>(index + 3));
	}

	// �u���b�N�̈ʒu�E��]�E�X�P�[���ݒ�
	for(auto& block : _vBlockPos)
	{
		// ���f���n���h���������Ȃ�X�L�b�v
		if(block.modelHandle < 0)
		{
			continue;
		}
		// �ʒu�E��]�E�X�P�[���ݒ�
		MV1SetPosition(block.modelHandle, VGet(block.x, block.y, block.z));
		MV1SetRotationXYZ(block.modelHandle, VGet(block.rx, block.ry, block.rz));
		MV1SetScale(block.modelHandle, VGet(block.sx, block.sy, block.sz));

		// �R���W�������� transform ��̏�ԂɍX�V�i�d�v�j
		MV1RefreshCollInfo(block.modelHandle, -1);
	}

	// SkySphere�i�X�P�[���E�ʒu�j�ݒ�
	if(_iHandleSkySphere >= 0)
	{
		// ��F200�{�i�K�v�Ȓl�ɒ����j
		float kSkySphereScale = 200.0f;
		MV1SetScale(_iHandleSkySphere, VGet(kSkySphereScale, kSkySphereScale, kSkySphereScale));

		// ���_�Œ肾�ƈړ��Œ[��������̂ŁA�J�����ɒǏ]������i�����j
		if(_cam)
		{
			MV1SetPosition(_iHandleSkySphere, DxlibConverter::VecToDxLib(_cam->GetPos()));
		}

		// �ϊ���̓����蔻��X�V���s�v�Ȃ�A���̍s�͂���܂���
		// MV1RefreshCollInfo(_iHandleSkySphere, -1);
	}

	return true;
}

// �`�揈��
bool Map1::Render()
{
	base::Render();

	// 3D��{�ݒ�
	SetUseZBuffer3D(TRUE);
	SetWriteZBuffer3D(TRUE);
	SetUseBackCulling(TRUE);

	// ���C�g�ݒ�
	const int extent = 800; // �V���h�E�}�b�v�͈̔�
	_mainLight.ApplyShadowMap(_iHandleShadowMap, DxlibConverter::VecToDxLib(_cam->GetTarget()), extent);

	// 2��܂킵�āApath = 0; �V���h�E�}�b�v�ւ̕`��Apath = 1; ���f���̕`��(�V���h�E�K�p�j	
	VECTOR lightdir = VGet(-1.0f, -1.0f, 0.5f);
#if 1 // ���s���C�g
	SetGlobalAmbientLight(GetColorF(0.f, 0.f, 0.f, 0.f));
	ChangeLightTypeDir(lightdir);
#endif
#if 0 // �|�C���g���C�g
	SetGlobalAmbientLight(GetColorF(0.f, 0.f, 0.f, 0.f));
	ChangeLightTypePoint(VAdd(_pos, VGet(0.50f, 0)), 1000.f, 0.f, 0.005f, 0.f);
#endif

	// �V���h�E�}�b�v���z�肷�郉�C�g�̕�����Z�b�g
	SetShadowMapLightDirection(_iHandleShadowMap, lightdir);

	// �V���h�E�}�b�v�ɕ`�悷��͈͂�ݒ�
	// �J�����̒����_�𒆐S�ɂ���
	float lenght = 800.f;
	DxlibConverter::SetShadowMapDrawArea(
		_iHandleShadowMap,
		vec3::VAdd(_cam->GetTarget(), vec3::VGet(-lenght, -1.0f, -lenght)),
		vec3::VAdd(_cam->GetTarget(), vec3::VGet(lenght, lenght, lenght))
	);
	// 2��܂킵�āApath = 0: �V���h�E�}�b�v�ւ̕`��Apath = 1: ���f���́F�`��
	for(int path = 0; path < 2; path++)
	{
		if(path == 0)
		{
			// �V���h�E�}�b�v�ւ̕`�揀��
			ShadowMap_DrawSetup(_iHandleShadowMap);

			// �V���h�E�L���X�^�[��`��(�X�J�C�͒ʏ�`���Ȃ�)
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
			// �V���h�E�}�b�v�ւ̕`��I��
			ShadowMap_DrawEnd();

			// �`��Ɏg�p����V���h�E�}�b�v��ݒ�
			SetUseShadowMap(0, _iHandleShadowMap);

			// ���C�g�ݒ������_���ɓK�p�i�A���r�G���g�E���C�g��ʓ��j
			_mainLight.ApplyRenderer();

			// �{�`��p�X�F�}�b�v�E�X�J�C�X�t�B�A�E�u���b�N��`��i�V���h�E�}�b�v���K�p�����j
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
	// �V���h�E�}�b�v�̉��
	SetUseShadowMap(0, -1);

	if(_ground_handle == -1)
	{
		return false;
	}

	// �n�ʂ�`��
	auto vertex_num = StCas<int>(_ground_vertex.size());
	auto index_num = StCas<int>(_ground_index.size());

	// �|���S����1��Ȃ���Ε`�悵�Ȃ�
	if(3 > vertex_num || 3 > index_num)
	{
		return false;
	}
	auto polygon_num = index_num / 3;
	DrawPolygonIndexed3D(_ground_vertex.data(), vertex_num, _ground_index.data(), polygon_num, _ground_handle, FALSE);

	for(auto& block : _vBlockPos)
	{
		// �u���b�N���f����`��
		MV1SetPosition(block.modelHandle, VGet(block.x, block.y, block.z));
		MV1SetRotationXYZ(block.modelHandle, VGet(block.rx, block.ry, block.rz));
		MV1SetScale(block.modelHandle, VGet(block.sx, block.sy, block.sz));
		MV1DrawModel(block.modelHandle);
	}
	return true;
}