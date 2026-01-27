#include "modeopscenario.h"
#include "modegame.h"

ModeOpScenario::ModeOpScenario()
{
	Initialize();

	Fade::GetInstance()->ColorMask(0, 0, 0, 0);		// カラーマスクの設定
	_state = ModeBase::State::WAIT;
	_fadeTimer = 0;
}

ModeOpScenario::~ModeOpScenario()
{
	Terminate();
}

bool ModeOpScenario::Initialize()
{
	ModeScenarioBase::Initialize();
	_soundServer = std::make_shared<soundserver::SoundServer>();

	_page =
	{
		{ LoadGraph(img::Op1), "" },// 画像読み込み, 音声ファイル
		{ LoadGraph(img::Op2), "" },
		{ LoadGraph(img::Op3), "" },
		{ LoadGraph(img::Op4), "" },
		{ LoadGraph(img::Op5), "" },
		{ LoadGraph(img::Op6), "" },
		{ LoadGraph(img::Op7), "" },
		{ LoadGraph(img::Op8), "" },
		{ LoadGraph(img::Op9), "" },
		{ LoadGraph(img::Op10), "" },
	};
	_pageNo = 0;

	if(!_page.empty())
	{
		_voice = new soundserver::SoundItemVoice(_page[_pageNo].voiceFile);
		_soundServer->Add("voice",_voice);
		_voice->Play();
	}

	return true;
}

bool ModeOpScenario::Terminate()
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

bool ModeOpScenario::Process()
{
	ModeBase::Process();

	ModeServer::GetInstance()->SkipProcessUnderLayer();
	ModeServer::GetInstance()->SkipRenderUnderLayer();



	// サウンド更新
	_soundServer->Update();

	int trg = ApplicationBase::GetInstance()->GetTrg();

	switch(_state)
	{
	case ModeBase::State::WAIT:
	{
		// 次のページへ
		if(trg & PAD_INPUT_2)
		{


			// 最終ページならフェードアウトへ
			if(_pageNo >= static_cast<int>(_page.size()) - 1)
			{
				_state = ModeBase::State::FADE_OUT;
				Fade::GetInstance()->FadeOut(0, 0, 0, FADE_FRAME);
			}
			else
			{
				ChangePage(+1);
			}
		}

		// 前のページへ
		if(trg & PAD_INPUT_1)
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
		ModeServer::GetInstance()->Add(new ModeGame(), 0, "game");
		ModeServer::GetInstance()->Del(this);
		break;
	}
	}

	return true;
}

bool ModeOpScenario::Render()
{
	ModeBase::Render();

	if(!_page.empty() && _page[_pageNo].handle != -1)
	{
		DrawGraph(0, 0, _page[_pageNo].handle, TRUE);
	}

	Fade::GetInstance()->Render();
	return true;
}
