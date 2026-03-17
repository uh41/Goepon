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
		// �Z�[�u�����݂���Γǂݍ���Ŋ����� ModeGame �ɓK�p�i������ "game" �͍폜���Ȃ��j
		SaveData sd{};
		if(SaveManager::TryLoad(sd, SaveManager::GetDefaultPath()))
		{
			ModeBase* existing = ModeServer::GetInstance()->Get("game");
			if(existing)
			{
				auto* game = dynamic_cast<ModeGame*>(existing);
				if(game)
				{
					game->ApplySaveData(sd);
					game->ResetEnemiesToInitialPositions(); // �G������ʒu�ɖ߂��i�Z�[�u�f�[�^�̈ʒu�ɍ��킹�邽�߁j
					// GameOver ���[�h��������i������ game ����̂܂܎c���j

					//game->ResetEnemyRoot(); // �G�̃��[�g����Z�b�g���ăZ�[�u�f�[�^�̈ʒu�ɍ��킹��
					ModeServer::GetInstance()->Del(this);
					return true;
				}
			}

			// ������ game ��������ΐV�K�������ăZ�[�u�̃X�e�[�W�ŋN��
			auto* newGame = new ModeGame();
			newGame->SetInitialStageId(sd.stageId);
			ModeServer::GetInstance()->Add(newGame, 0, "game");
			ModeServer::GetInstance()->ProcessInit();

			// Initialize ��Ɏ擾���ăZ�[�u��e��K�p�iApplySaveData �̓^�k�L�J�n�ɌŒ肷��j
			{
				ModeBase* gm = ModeServer::GetInstance()->Get("game");
				if(gm)
				{
					auto* game = dynamic_cast<ModeGame*>(gm);
					if(game)
					{
						game->ApplySaveData(sd);
						game->ResetEnemiesToInitialPositions();
						//game->ResetEnemyRoot();
					}
				}
			}

			ModeServer::GetInstance()->Del(this);
			return true;
		}

		// �Z�[�u��������Ώ]���̋����փt�H�[���o�b�N�i������ game ��폜���Ȃ��j
		if(ModeServer::GetInstance()->Get("gameoverload") == nullptr)
		{
			ModeServer::GetInstance()->Add(new ModeGameOverLoad(nullptr, _debugCurrentStageId), 300, "gameoverload");
			ModeServer::GetInstance()->ProcessInit();
		}
		ModeServer::GetInstance()->Del(this);
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