#include "modeopscenario.h"
#include "modegame.h"
#include "modegameload.h"

#ifdef _DEBUG
#include <crtdbg.h>
#define NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
#else
#define NEW new
#endif

ModeOpScenario::ModeOpScenario()
{
	Initialize();

	Fade::GetInstance()->ColorMask(0, 0, 0, 0);		// �J���[�}�X�N�̐ݒ�
	_state = ModeBase::State::WAIT;
	_fadeTimer = 0;
}

ModeOpScenario::~ModeOpScenario()
{
	Terminate();
}

bool ModeOpScenario::Initialize()
{
	base::Initialize();
	_soundServer = std::make_shared<soundserver::SoundServer>();

	_backHandle = LoadGraph("res/Prologue/BG_opstory.png");	// �w�i�摜�̓ǂݍ���

	_page =
	{
		{ LoadGraph("res/Prologue/1.png"), ""},// �摜�ǂݍ���, �����t�@�C��
		{ LoadGraph("res/Prologue/2.png"), ""},// �摜�ǂݍ���, �����t�@�C��
		{ LoadGraph("res/Prologue/3.png"), ""},// �摜�ǂݍ���, �����t�@�C��
		{ LoadGraph("res/Prologue/4.png"), ""},// �摜�ǂݍ���, �����t�@�C��
		{ LoadGraph("res/Prologue/5.png"), ""},// �摜�ǂݍ���, �����t�@�C��
		{ LoadGraph("res/Prologue/6.png"), ""},// �摜�ǂݍ���, �����t�@�C��
		{ LoadGraph("res/Prologue/7.png"), ""},// �摜�ǂݍ���, �����t�@�C��
		{ LoadGraph("res/Prologue/8.png"), ""},// �摜�ǂݍ���, �����t�@�C��
		{ LoadGraph("res/Prologue/9.png"), ""},// �摜�ǂݍ���, �����t�@�C��
		{ LoadGraph("res/Prologue/10.png"), ""},// �摜�ǂݍ���, �����t�@�C��
	};

	// �y�[�W�̕\���f�[�^��ݒ�
	_panelData =
	{
		{ 50,  50,  250, 250, 0.0f },   // �y�[�W1�̈ʒu�E�T�C�Y�E�����x
		{ 50, 200,  250, 250, 0.0f },   // �y�[�W2
		{ 50,  450, 250, 250, 0.0f },   // �y�[�W3
		{ 300,  50, 250, 250, 0.0f },   // �y�[�W4
		{ 300, 300, 250, 250, 0.0f },   // �y�[�W5
		{ 300, 500, 250, 250, 0.0f },   // �y�[�W6
		{ 550,  50, 300, 300, 0.0f },   // �y�[�W7
		{ 550, 340, 350, 350, 0.0f },   // �y�[�W8
		{ 900,  50, 400, 400, 0.0f },   // �y�[�W9
		{ 920, 450, 300, 300, 0.0f },   // �y�[�W10
	};
	_pageNo = 0;
	_fadeTimer = 0;

	if(!_page.empty())
	{
		_voice = std::make_shared<soundserver::SoundItemVoice>(_page[_pageNo].voiceFile);
		_soundServer->Add("voice",_voice);
		_voice->Play();
	}

	return true;
}

bool ModeOpScenario::Terminate()
{
	base::Terminate();

	return true;
}

bool ModeOpScenario::Process()
{
	base::Process();

	ModeServer::GetInstance()->SkipProcessUnderLayer();
	ModeServer::GetInstance()->SkipRenderUnderLayer();

	// �T�E���h�X�V
	_soundServer->Update();

	// �t�F�[�h�C������
	if(_pageNo >= 0 && _pageNo < StCas<int>(_panelData.size()))
	{
		if(_panelData[_pageNo].alpha < 1.0f)
		{
			_fadeTimer++;
			_panelData[_pageNo].alpha = StCas<float>(_fadeTimer) / FADE_FRAME;
			// �����x��1.0f�𒴂��Ȃ��悤�ɂ���	
			if(_panelData[_pageNo].alpha > 1.0f)
			{
				_panelData[_pageNo].alpha = 1.0f;
			}
		}
	}
	int trg = ApplicationBase::GetInstance()->GetTrg();

	switch(_state)
	{
		case ModeBase::State::WAIT:
		{
			if(trg & PAD_INPUT_2)
			{
				if (_pageNo >= 0 && _pageNo < StCas<int>(_panelData.size()))
				{
					_panelData[_pageNo].alpha = 1.0f;
				}

				// ���̃y�[�W������Ȃ�i�߂�
				if(_pageNo < StCas<int>(_page.size()) - 1)
				{
					_pageNo++;
					_fadeTimer = 0;

					// ����������΍Đ�
					if(!_page[_pageNo].voiceFile.empty())
					{
						if (_soundServer)
						{
							_soundServer->StopType(soundserver::SoundItemBase::TYPE::VOICE);
							_voice = std::make_shared<soundserver::SoundItemVoice>(_page[_pageNo].voiceFile);
							_soundServer->Add("voice", _voice);
							_voice->Play();
						}
					}
				}
				else
				{
					// �ŏI�y�[�W�Ȃ̂Ńt�F�[�h�A�E�g�J�n
					_state = ModeBase::State::FADE_OUT;
					Fade::GetInstance()->FadeOut(0, 0, 0, FADE_FRAME);	// �t�F�[�h�A�E�g�J�n
				}
			}
			if(trg & PAD_INPUT_1)
			{
				_state = ModeBase::State::FADE_OUT;
				Fade::GetInstance()->FadeOut(0, 0, 0, FADE_FRAME);
			}
			break;
		}
		case ModeBase::State::FADE_OUT:
		{
			Fade::GetInstance()->Process();
			if(Fade::GetInstance()->IsFade() == false)
			{
				_state = ModeBase::State::DONE;
			}
			break;
		}
		case ModeBase::State::DONE:
		{
			if(_soundServer)
			{
				_soundServer->Clear();
				_soundServer = nullptr;
			}
			_voice = nullptr;

			// ���̃��[�h��
			ModeServer::GetInstance()->Add(NEW ModeGameLoad(), 0, "ModeGameLoad");
			ModeServer::GetInstance()->Del(this);
			break;
		}
	}
	return true;
}

bool ModeOpScenario::Render()
{
	base::Render();

	// �w�i�̕`��
	if(_backHandle != -1)
	{
		DrawGraph(0, 0, _backHandle, TRUE);
	}

	// �\������Ă���R�}����Ԃɕ`��
	for(int i = 0; i <= _pageNo && i < StCas<int>(_page.size()); i++)
	{
		if(_page[i].handle != -1 && i < StCas<int>(_panelData.size()))
		{
			// �R�}�𖾂邭����G�t�F�N�g��ǉ�
			SetDrawBlendMode(DX_BLENDMODE_ADD, 30); // ���Z�u�����h�ŏ������邭
			// �ʏ�̕`��
			SetDrawBlendMode(DX_BLENDMODE_ALPHA, StCas<int>(255 * _panelData[i].alpha));
			DrawExtendGraph
			(
				_panelData[i].x,
				_panelData[i].y,
				_panelData[i].x + _panelData[i].w,
				_panelData[i].y + _panelData[i].h,
				_page[i].handle,
				TRUE
			);
			SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
		}
	}
	Fade::GetInstance()->Render();	// �t�F�[�h�`��
	return true;
}
