#include "modetitle.h"
#include "modeloading.h"

#ifdef _DEBUG
#include <crtdbg.h>
#define NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
#else
#define NEW new
#endif

ModeTitle::ModeTitle()
{
	Initialize();
}

ModeTitle::~ModeTitle()
{
	Terminate();
}

bool ModeTitle::Initialize()
{
	// �w�i�摜��ǂݍ��ށi�p�X�̓v���W�F�N�g�̃��\�[�X�z�u�ɍ��킹�ĕύX�j
	_handle = LoadGraph("res/Title/BG_op.png");
	if(_handle == -1)
	{
		return false;
	}

	// �K�L�����f�[�^��쐬
	_player = std::make_shared<TitleTanuki>();
	if(_player)
	{
		_player->Initialize();
		// �Œ�ʒu�ɔz�u�iY���W��K�؂ɐݒ�j
		_player->SetPos(vec::Vec3(0.0f, 0.0f, 0.0f));
		_player->SetDir(vec::Vec3(0.0f, 0.0f, 1.0f));
	}

	// �^�C�g���p�̃J������쐬
	_cam = new TitleCamera();
	if(_cam)
	{
		_cam->Initialize();
	}

	// �t�F�[�h�Ȃ� - �����ɕ\��
	_state = ModeBase::State::WAIT;

	return true;
}

bool ModeTitle::Terminate()
{
	if(_handle != -1)
	{
		DeleteGraph(_handle);
		_handle = -1;
	}

	// �K�L�����J��
	if(_player)
	{
		_player->Terminate();
		_player.reset();
	}

	// �J�����J��
	if(_cam)
	{
		delete _cam;
		_cam = nullptr;
	}

	return true;
}

bool ModeTitle::Process()
{
	// �^�C�g���͍őO�ʃ��C���[�Ƃ��ĉ�ʂ̏����ƕ`��X�L�b�v
	ModeServer::GetInstance()->SkipProcessUnderLayer();
	ModeServer::GetInstance()->SkipRenderUnderLayer();

	int trg = ApplicationBase::GetInstance()->GetTrg();

	// �J�����ƃv���C���[�̏���
	if(_cam) { _cam->Process(); }
	if(_player) { _player->Process(); }

	// �X�e�[�g����
	switch(_state)
	{
		case ModeBase::State::WAIT:
			// �{�^�����������܂őҋ@
			if(trg & PAD_INPUT_2)
			{
				_state = ModeBase::State::DONE;
			}
			break;
		case ModeBase::State::DONE:
			// ���̃��[�h�ֈڍs
			ModeServer::GetInstance()->Add(NEW ModeLoading(), 1, "loading");
			ModeServer::GetInstance()->Del(this);
			break;
	}

	return true;
}

bool ModeTitle::Render()
{
	if(!_cam)
	{
		return false;
	}

	base::Render();

	// �w�i�摜�`��
	if(_handle != -1)
	{
		DrawGraph(0, 0, _handle, TRUE);
	}

	// �J�����ݒ�X�V
	SetCameraPositionAndTarget_UpVecY(DxlibConverter::VecToDxLib(_cam->GetPos()), DxlibConverter::VecToDxLib(_cam->GetTarget()));
	SetCameraNearFar(_cam->GetClipNear(), _cam->GetClipFar());

	// ����p�ݒ�
	float fov_deg = 30.0f;
	float fov_rad = DEG2RAD(fov_deg);
	SetupCamera_Perspective(fov_rad);

	// �K���f���`��
	if(_player)
	{
		_player->Render();
	}

	return true;
}