#include "ModeGameOver.h"
#include "ApplicationMain.h"
#include "modegame.h"
#include "modegameoverload.h"

bool ModeGameOver::Initialize()
{
	if(!base::Initialize()) return false;

	_debugCurrentStageId = "Stage1"; // �f�t�H���g
	if(_ownerGame)
	{
		auto* game = dynamic_cast<ModeGame*>(_ownerGame);
		if(game)
		{
			_debugCurrentStageId = game->GetCurrentStageId();
		}
	}

	return true;
}

bool ModeGameOver::Terminate()
{
	// �����`��p�Ƀ��[�h�����O���t����
	if(_overlayHandle != -1)
	{
		DeleteGraph(_overlayHandle);
		_overlayHandle = -1;
		_showOverlayImmediate = false;
	}

	base::Terminate();
	return true;
}

bool ModeGameOver::Process()
{
	base::Process();
	ModeServer::GetInstance()->SkipProcessUnderLayer();

	//ModeServer::GetInstance()->SkipProcessUnderLayer();
	int trg = ApplicationMain::GetInstance()->GetTrg();

	if(trg & PAD_INPUT_1)
	{
		// 1) ���݂̃X�e�[�WID��ۑ��i�폜�O�Ɏ擾�j
		std::string currentStageId = "Stage1"; // �f�t�H���g�l
		if(_ownerGame)
		{
			auto* game = dynamic_cast<ModeGame*>(_ownerGame);
			if(game)
			{
				currentStageId = game->GetCurrentStageId();
				// �f�o�b�O�o�́F�擾�����X�e�[�WID
				//DrawFormatString(10, 100, GetColor(255, 0, 0), "DEBUG: Current Stage ID = %s", currentStageId.c_str());
			}
			/*else
			{
				//DrawFormatString(10, 100, GetColor(255, 0, 0), "DEBUG: Failed to cast _ownerGame to ModeGame");
			}
		}
		/*else
		{
			//DrawFormatString(10, 100, GetColor(255, 0, 0), "DEBUG: _ownerGame is null, using default Stage1");
		}
		// 2) ���L���Ă��� ModeGame ������΍폜�\��i���S�ɗ\�񂷂�j
		if(_ownerGame)
		{
			ModeServer::GetInstance()->Del(_ownerGame);
			_ownerGame = nullptr; // ���L�Q�Ƃ�؂�
		}

		// 3) ���O "game" �œo�^����Ă��郂�[�h������΍폜�\��
		ModeBase* existing = ModeServer::GetInstance()->Get("game");
		if(existing)
		{
			ModeServer::GetInstance()->Del(existing);
		}

		// �f�o�b�O�o�́F�ŏI�I�Ɏg�p����X�e�[�WID
		//DrawFormatString(10, 140, GetColor(255, 255, 0), "DEBUG: Final Stage ID = %s", currentStageId.c_str());

		// 4) �I�[�o�[���C��\��ǉ����Ɍ��݂̃X�e�[�WID��n��
		if(ModeServer::GetInstance()->Get("gameoverload") == nullptr)
		{
			ModeServer::GetInstance()->Add(new ModeGameOverLoad(nullptr, currentStageId), 300, "gameoverload");
			ModeServer::GetInstance()->ProcessInit();
		}

		// 5) �������g��폜�\��
		ModeServer::GetInstance()->Del(this);

		// �폜�E�ǉ��͎��t���[���� ModeServer::ProcessInit() �Ŏ��s����邽�߁A
		// �����ł͑������^�[�����Ĉ��S�ɏI������B
		return true;
	}

	return true;
}

bool ModeGameOver::Render()
{
	base::Render();

	// �f�o�b�O���̕\���iRender��ōs���j
	//DrawFormatString(10, 50, GetColor(255, 255, 0), "DEBUG: Detected Stage ID = %s", _debugCurrentStageId.c_str());
	//DrawFormatString(10, 70, GetColor(255, 255, 0), "DEBUG: _ownerGame = %s", _ownerGame ? "Valid" : "NULL");

	// �w�i(�������ɐݒ�)
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, BackgroundAlpha);
	DrawBox
	(
		BgLeft, BgTop, BgRight, BgBottom,
		GetColor(BlackR, BlackG, BlackB),
		TRUE
	);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

	// �g
	DrawBox
	(
		BgLeft, BgTop, BgRight, BgBottom,
		GetColor(255, 255, 255),
		FALSE
	);

	// �Q�[���I�[�o�[���b�Z�[�W
	SetFontSize(TitleFontSize);
	DrawString
	(
		TitlePosX, TitlePosY,
		GameOverMessage,
		GetColor(WhiteR, WhiteG, WhiteB)
	);

	// �q���g���b�Z�[�W
	SetFontSize(HintFontSize);
	DrawString
	(
		HintPosX, HintPosY,
		HintMessage,
		GetColor(HintR, HintG, HintB)
	);

	// PAD ��������̑����\���iModeGameOverLoad ���܂�����������Ă��Ȃ��t���[�������j
	if(_showOverlayImmediate && _overlayHandle != -1)
	{
		int screenW, screenH;
		GetScreenState(&screenW, &screenH, nullptr);

		int imgW, imgH;
		GetGraphSize(_overlayHandle, &imgW, &imgH);

		int x = (screenW - imgW) / 2;
		int y = (screenH - imgH) / 2;

		// overlay ��O�ʂɕ`��
		DrawGraph(x, y, _overlayHandle, TRUE);
	}

	return true;
}