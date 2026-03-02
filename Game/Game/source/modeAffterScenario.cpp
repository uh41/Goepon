#include "modeAffterScenario.h"
#include "modegame.h"
#include "modeteamlogo.h"

#ifdef _DEBUG
#include <crtdbg.h>
#define NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
#else
#define NEW new
#endif

ModeAfScenario::ModeAfScenario()
{
	Initialize();

	Fade::GetInstance()->ColorMask(0, 0, 0, 0);		// カラーマスクの設定
	_state = ModeBase::State::WAIT;
	_fadeTimer = 0;
}

ModeAfScenario::~ModeAfScenario()
{
	Terminate();
}

bool ModeAfScenario::Initialize()
{
	ModeScenarioBase::Initialize();
	_soundServer = std::make_shared<soundserver::SoundServer>();

	_page =
	{
		{ LoadGraph(img::Op1), "" },// 画像読み込み, 音声ファイル
	};
	_pageNo = 0;

	if(!_page.empty())
	{
		_voice = std::make_shared<soundserver::SoundItemVoice>(_page[_pageNo].voiceFile);
		_soundServer->Add("voice", _voice);
		_voice->Play();
	}

	return true;
}

bool ModeAfScenario::Terminate()
{
	ModeBase::Terminate();

	if(_soundServer)
	{
		_soundServer->StopType(soundserver::SoundItemBase::TYPE::VOICE);
		_soundServer->Clear();
		_soundServer = nullptr;
	}

	for(auto& page : _page)
	{
		if(page.handle != -1)
		{
			DeleteGraph(page.handle);
			page.handle = -1;
		}
	}
	_page.clear();

	return true;
}

bool ModeAfScenario::Process()
{
	ModeServer::GetInstance()->SkipProcessUnderLayer();
	ModeServer::GetInstance()->SkipRenderUnderLayer();

	// サウンド更新
	_soundServer->Update();

	int trg = ApplicationBase::GetInstance()->GetTrg();

	switch(_state)
	{
		case ModeGame::State::WAIT:
		{
			// 次のページへ
			if (trg & PAD_INPUT_2)
			{
				// 最終ページならフェードアウト
				if (_pageNo >= StCas<int>(_page.size()) - 1)
				{
					_state = ModeGame::State::FADE_OUT;
					Fade::GetInstance()->FadeOut(0, 0, 0, FADE_FRAME);	// フェードアウト開始
				}
				else
				{
					ChangePage(1);
				}
			}

			// 前のページに行く
			if (trg & PAD_INPUT_1)
			{
				ChangePage(-1);
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
			// 次のモードへ移行
			ModeServer::GetInstance()->Add(NEW ModeTeamLogo(), 2, "teamlogo");
			ModeServer::GetInstance()->Del(this);
			break;
		}
	}
	return true;
}

bool ModeAfScenario::Render()
{
	ModeBase::Render();

	if (!_page.empty() && _page[_pageNo].handle != -1)
	{
		DrawGraph(0, 0, _page[_pageNo].handle, TRUE);
	}

	Fade::GetInstance()->Render();
	return true;
}
