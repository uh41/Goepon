#include "Map1.h"
#include "applicationglobal.h"

// ������
bool Map1::Initialize()
{
	// ���N���X������
	if(!base::Initialize()) { return false; }

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

	return true;
}

// �I��
bool Map1::Terminate()
{
	// 基底クラスの終了処理
	base::Terminate();
	return true;
}

// �X�V����
bool Map1::Process()
{
	base::Process();
	return true;
}

// �`�揈��
bool Map1::Render()
{
	base::Render();
	return true;
}