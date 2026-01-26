#include "modeinit.h"
#include "modeteamlogo.h"

ModeInit::ModeInit()
{
	Initialize();

	Fade::GetInstance()->ColorMask(0, 0, 0, 255);		// カラーマスクの設定
	Fade::GetInstance()->FadeIn(FADE_FRAME);	// フェードイン開始

	_state = ModeBase::State::FADE_IN;
	_fadeTimer = 0;
}

ModeInit::~ModeInit()
{
	Terminate();
}

bool ModeInit::Initialize()
{
	_handle = LoadGraph(img::AMGlogo);
	_isWait = false;
	
	return true;
}

bool ModeInit::Terminate()
{
	if(_handle != -1)
	{
		DeleteGraph(_handle);
		_handle = -1;
	}
	return true;
}

bool ModeInit::Process()
{
	// フェード処理
	Fade::GetInstance()->Process();

	ModeServer::GetInstance()->SkipProcessUnderLayer();
	ModeServer::GetInstance()->SkipRenderUnderLayer();

	switch(_state)
	{
		case ModeBase::State::FADE_IN:
		{
			if(Fade::GetInstance()->IsFade() == false)
			{
				_state = ModeBase::State::WAIT;
				_fadeTimer = FADE_WAIT;
			}
			break;
		}
		case ModeBase::State::WAIT:
		{
			_fadeTimer--;
			if(_fadeTimer <= 0)
			{
				_state = ModeBase::State::FADE_OUT;
				Fade::GetInstance()->FadeOut(0, 0, 0, FADE_FRAME);
			}
			break;
		}
		case ModeBase::State::FADE_OUT:
		{
			if(Fade::GetInstance()->IsFade() == false)
			{
				_state = ModeBase::State::DONE;
			}
			break;
		}
		case ModeBase::State::DONE:
		{
			ModeServer::GetInstance()->Del(this);
			ModeServer::GetInstance()->Add(new ModeTeamLogo(), 2, "teamlogo");

			break;
		}
	}
	return true;
}

bool ModeInit::Render()
{
	DrawGraph(0, 0, _handle, TRUE);

	Fade::GetInstance()->Render();
	return true;
}

