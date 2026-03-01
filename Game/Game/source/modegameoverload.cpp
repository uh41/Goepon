#include "modegameoverload.h"
#include "ModeGame.h"
#include "applicationglobal.h"

ModeGameOverLoad::ModeGameOverLoad(ModeBase* ownerGame, const std::string& stageId)
{
	_owner = ownerGame;
	_stageId = stageId.empty() ? "Stage1" : stageId; // �R���X�g���N�^�Őݒ�
	_handle = -1;
	_frameShow = 0;
	_requestedReset = false;
	_spawnedGame = false;
}


ModeGameOverLoad::~ModeGameOverLoad()
{

}

bool ModeGameOverLoad::Initialize()
{
	if(!base::Initialize()) return false;
	_handle = LoadGraph(img::gameoverload);

	return true;
}

bool ModeGameOverLoad::Terminate()
{
	base::Terminate();
	if(_handle != -1)
	{
		DeleteGraph(_handle);
		_handle = -1;
	}
	return true;
}

bool ModeGameOverLoad::Process()
{
	base::Process();

	// ModeGame���ۑ����邩�m�F
	ModeBase* gameBase = ModeServer::GetInstance()->Get("game");
	if(gameBase == nullptr)
	{
		// game���Ȃ���Έ�x�����V�����ǉ����ă��[�h��J�n
		if(!_spawnedGame)
		{
			// �ۑ����ꂽ�X�e�[�WID��g�p���ĐV����ModeGame��쐬
			auto* newGame = new ModeGame();
			newGame->SetInitialStageId(_stageId);

			ModeServer::GetInstance()->Add(newGame, 0, "game");
			ModeServer::GetInstance()->ProcessInit(); // �ǉ��������[�h��Initialize��Ăяo��
			_spawnedGame = true;
		}
		// �ǉ�����̓��[�h���i�ނ܂ő҂�
		return true;
	}
	auto* game = dynamic_cast<ModeGame*>(gameBase);
	if(game == nullptr) return false;

	// �V�����ǉ����ꂽ ModeGame �����[�h����������I�[�o�[���C��폜����
	if(game->IsLoadComplete())
	{
		ModeServer::GetInstance()->Del(this); // �������g��폜�\��	
	}
	return true;
}

bool ModeGameOverLoad::Render()
{
	base::Render();

	_frameShow++;

	// �f�o�b�O: Render �Ăяo���m�F�\���i��ʍ���j
	DrawFormatString(10, 10, GetColor(255, 255, 0), "ModeGameOverLoad Render frame=%d handle=%d", _frameShow, _handle);

	if(_handle != -1)
	{
		// �摜���ʒ����ɕ`��
		int screenW, screenH;
		GetScreenState(&screenW, &screenH, nullptr);

		int imgW, imgH;
		GetGraphSize(_handle, &imgW, &imgH);

		// �f�o�b�O: �摜�T�C�Y�\��
		DrawFormatString(10, 30, GetColor(255, 255, 0), "imgW=%d imgH=%d", imgW, imgH);

		int x = (screenW - imgW) / 2;
		int y = (screenH - imgH) / 2;

		DrawGraph(x, y, _handle, TRUE);
	}
	else
	{
		// �t�H�[���o�b�N�\���i�f�o�b�O�p�j
		DrawString(10, 10, "GameOver overlay missing", GetColor(255, 0, 0));
	}
	return true;
}