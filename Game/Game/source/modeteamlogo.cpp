#include "modeteamlogo.h"
#include "modetitle.h"

ModeTeamLogo::ModeTeamLogo()
{
	Initialize();

	Fade::GetInstance()->ColorMask(0, 0, 0, 255);		// カラーマスクの設定
	Fade::GetInstance()->FadeIn(FADE_FRAME);	// フェードイン開始

	_state = ModeBase::State::FADE_IN;
	_fadeTimer = 0;
}

ModeTeamLogo::~ModeTeamLogo()
{
	Terminate();
}

bool ModeTeamLogo::Initialize()
{
	_handle = LoadGraph(img::teamlogo);
	_isWait = false;
	return true;
}

bool ModeTeamLogo::Terminate()
{
	if(_handle != -1)
	{
		DeleteGraph(_handle);
		_handle = -1;
	}
	return true;
}

bool ModeTeamLogo::Process()
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
				Fade::GetInstance()->FadeOut(0, 0, 0, FADE_FRAME);	// フェードアウト開始
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
			// 次のモードへ移行
			ModeServer::GetInstance()->Add(new ModeTitle(), 1, "game");
			ModeServer::GetInstance()->Del(this);

			break;
		}
	}
	return true;
}

bool ModeTeamLogo::Render()
{
	// 画面中央にロゴを描画

	DrawGraph(0, 0, _handle, TRUE);
	// フェード描画
	Fade::GetInstance()->Render();
	return true;
}
