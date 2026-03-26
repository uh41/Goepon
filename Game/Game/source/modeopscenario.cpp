#include "modeopscenario.h"
#include "modegame.h"
#include "modegameload.h"
#include "mymath.h"
#ifdef _DEBUG
#include <crtdbg.h>
#define NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
#else
#define NEW new
#endif

ModeOpScenario::ModeOpScenario()
{
	Initialize();
	_state = ModeBase::State::WAIT;
	_fadeTimer = 0;
}

ModeOpScenario::~ModeOpScenario()
{
	Terminate();
}

bool ModeOpScenario::Initialize()
{
	Fade::GetInstance()->ColorMask(0, 0, 0, 0);
	base::Initialize();
	_soundServer = std::make_shared<soundserver::SoundServer>();

	//_backHandle = LoadGraph("res/Prologue/BG_opstory.png");	// �w�i�摜�̓ǂݍ���

	_page =
	{
		{ LoadGraph("res/Prologue/1.png"), mp3::UI_Henshin_pon},// �摜�ǂݍ���, �����t�@�C��
		{ LoadGraph("res/Prologue/2.png"), mp3::UI_Henshin_pon},// �摜�ǂݍ���, �����t�@�C��
		{ LoadGraph("res/Prologue/3.png"), mp3::UI_Henshin_pon},// �摜�ǂݍ���, �����t�@�C��
		{ LoadGraph("res/Prologue/4.png"), mp3::UI_Henshin_pon},// �摜�ǂݍ���, �����t�@�C��
		{ LoadGraph("res/Prologue/5.png"), mp3::UI_Henshin_pon},// �摜�ǂݍ���, �����t�@�C��
		{ LoadGraph("res/Prologue/6.png"), mp3::UI_Henshin_pon},// �摜�ǂݍ���, �����t�@�C��
		{ LoadGraph("res/Prologue/7.png"), mp3::UI_Henshin_pon},// �摜�ǂݍ���, �����t�@�C��
		{ LoadGraph("res/Prologue/8.png"), mp3::UI_Henshin_pon},// �摜�ǂݍ���, �����t�@�C��
		{ LoadGraph("res/Prologue/9.png"), mp3::UI_Henshin_pon},// �摜�ǂݍ���, �����t�@�C��
		{ LoadGraph("res/Prologue/10.png"), mp3::UI_Henshin_pon},// �摜�ǂݍ���, �����t�@�C��
		{ LoadGraph("res/Prologue/11.png"), mp3::UI_Henshin_pon},// �摜�ǂݍ���, �����t�@�C��
		{ LoadGraph("res/Prologue/12.png"), mp3::UI_Henshin_pon},// �摜�ǂݍ���, �����t�@�C��
		{ LoadGraph("res/Prologue/13.png"), mp3::UI_Henshin_pon},// �摜�ǂݍ���, �����t�@�C��
		{ LoadGraph("res/Prologue/14.png"), mp3::UI_Henshin_pon},// �摜�ǂݍ���, �����t�@�C��
		{ LoadGraph("res/Prologue/15.png"), mp3::UI_Henshin_pon},// �摜�ǂݍ���, �����t�@�C��
		{ LoadGraph("res/Prologue/16.png"), mp3::UI_Henshin_pon},// �摜�ǂݍ���, �����t�@�C��
		{ LoadGraph("res/Prologue/17.png"), mp3::UI_Henshin_pon},// �摜�ǂݍ���, �����t�@�C��
		{ LoadGraph("res/Prologue/18.png"), mp3::UI_Henshin_pon},// �摜�ǂݍ���, �����t�@�C��
	};

	// �y�[�W�̕\���f�[�^��ݒ�
	_panelData.clear();
	_panelData.resize(_page.size());
	for(int i = 0; i < StCas<int>(_panelData.size()); ++i)
	{
		_panelData[i].x = 0;
		_panelData[i].y = 0;
		_panelData[i].alpha = 0.0f;
	}
	_pageNo = 0;
	_fadeTimer = 0;

	//if(!_page.empty())
	//{
	//	_se = std::make_shared<soundserver::SoundItemSE>(_page[_pageNo].voiceFile);
	//	_soundServer->Add("se",_se);
	//	_se->Play();
	//}

	if(gGlobal._soundServer)
	{
		auto bgm = gGlobal._soundServer->Get("110");
		if(bgm)
		{
			bgm->Stop();
			bgm->Play();
		}
	}

	_moveHandle = LoadGraph(ui::prologe_config);

	// ふすまの初期化(開く前)
	_fusumaRighetHandle = LoadGraph(img::Fusuma_R);
	_fusumaLeftHandle = LoadGraph(img::Fusuma_L);

	if (_fusumaLeftHandle != -1)
	{
		GetGraphSize(_fusumaLeftHandle, &_fusumaW, &_fusumaH);
	}
	_fusumaY            = 0.0f;
	const float screenW = 1920.0f;
	_fusumaLeftX        = 0.0f;
	_fusumaRightX       = screenW - StCas<float>(_fusumaW);

	StartFusumaOpen();
	return true;
}

bool ModeOpScenario::Terminate()
{
	base::Terminate();

	if(gGlobal._soundServer)
	{
		auto bgm = gGlobal._soundServer->Get("110");
		if(bgm)
		{
			bgm->Stop();
		}
	}

	return true;
}

bool ModeOpScenario::Process()
{
	base::Process();

	// 裏ロード
	gGlobal.UpdateLoadProgress();

	ProcessFusumaOpen();

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
			if(trg & PAD_INPUT_1)
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
							_soundServer->StopType(soundserver::SoundItemBase::TYPE::SE);
							_se = std::make_shared<soundserver::SoundItemSE>(_page[_pageNo].voiceFile);
							_soundServer->Add("se", _se);
							_se->Play();
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
			if(trg & PAD_INPUT_2)
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
			_se = nullptr;

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
			DrawGraph(
				_panelData[i].x,
				_panelData[i].y,
				_page[i].handle,
				TRUE
			);
			SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
		}
	}

	DrawGraph(ui::MOVE_X, ui::MOVE_Y, _moveHandle, TRUE);

	RenderFusuma();

	Fade::GetInstance()->Render();	// �t�F�[�h�`��
	return true;
}

void ModeOpScenario::StartFusumaOpen()
{
	_fusumaState = FusumaState::Opening;
	_fusumaCnt = 0.0f;
}

bool ModeOpScenario::ProcessFusumaOpen()
{
	if (_fusumaState != FusumaState::Opening)
	{
		return false;
	}

	_fusumaCnt += 1.0f;
	if (_fusumaCnt > _fusumaFrames)
	{
		_fusumaCnt = _fusumaFrames;
	}

	const float screenW = 1920.0f;

	// 開始：閉じている位置
	const float leftStartX = 0.0f;
	const float rightStartX = screenW - StCas<float>(_fusumaW);

	// 終了：画面外へ退避（開いた状態）
	const float leftEndX = -StCas<float>(_fusumaW);
	const float rightEndX = screenW;

	_fusumaLeftX  = mymath::EasingLinear(_fusumaCnt, leftStartX, leftEndX, _fusumaFrames);
	_fusumaRightX = mymath::EasingLinear(_fusumaCnt, rightStartX, rightEndX, _fusumaFrames);

	if (_fusumaCnt >= _fusumaFrames)
	{
		_fusumaState = FusumaState::Opened;
		return true;
	}

	return false;
}

void ModeOpScenario::RenderFusuma() const
{
	if (_fusumaLeftHandle == -1 || _fusumaRighetHandle == -1)
	{
		return;
	}

	// Opening中だけ描画（Openedになったら描かない）
	if (_fusumaState == FusumaState::Opened || _fusumaState == FusumaState::None)
	{
		return;
	}

	DrawGraph(StCas<int>(_fusumaLeftX), StCas<int>(_fusumaY), _fusumaLeftHandle, TRUE);
	DrawGraph(StCas<int>(_fusumaRightX), StCas<int>(_fusumaY), _fusumaRighetHandle, TRUE);
}