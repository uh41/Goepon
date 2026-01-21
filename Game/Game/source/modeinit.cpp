#include "modeinit.h"
#include "modetitle.h"

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
	_iHandle = LoadGraph(img::amglogo);
	_isWait = false;
	
	return true;
}

bool ModeInit::Terminate()
{
	if(_iHandle != -1)
	{
		DeleteGraph(_iHandle);
		_iHandle = -1;
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
				ModeServer::GetInstance()->Add(new ModeTitle(), ModeServer::GetInstance()->LayerTop(), "title");
				ModeServer::GetInstance()->Del(this);
				_state = ModeBase::State::DONE;
			}
			break;
		}
		case ModeBase::State::DONE:
		{
		default:
			break;
		}
	}
	return true;
}

bool ModeInit::Render()
{
	DrawGraph(0, 0, _iHandle, TRUE);

	Fade::GetInstance()->Render();
	return true;
}

