#include "modecredit.h"
#include "modeinit.h"
#include "applicationglobal.h"

ModeCredit::ModeCredit()
{
	Initialize();

	// フェードインは行わずすぐ再生するため、状態は WAIT にする
	Fade::GetInstance()->ColorMask(0, 0, 0, 0);
	_state = ModeBase::State::WAIT;
	_fadeTimer = 0;
}

bool ModeCredit::Initialize()
{
	_handle = MovieManager::GetInstance()->LoadMovie(mp4::EndCredits);

	_creditHandle = LoadGraph(ui::Staffroll_config);

	// ロード成功していれば即再生する
	if(_handle != -1)
	{
		MovieManager::GetInstance()->PlayMovie(_handle);

		if(gGlobal._soundServer)
		{
			auto bgm = gGlobal._soundServer->Get("74");
			if(bgm)
			{
				bgm->Play();
			}
		}
	}

	return true;
}

bool ModeCredit::Terminate()
{
	if(_handle != -1)
	{
		MovieManager::GetInstance()->UnloadMovie(_handle);
		_handle = -1;
	}

	if(gGlobal._soundServer)
	{
		auto bgm = gGlobal._soundServer->Get("74");
		if(bgm)
		{
			bgm->Stop();
		}
	}
	return true;
}

bool ModeCredit::Process()
{
	//ModeServer::GetInstance()->SkipProcessUnderLayer();
	//ModeServer::GetInstance()->SkipRenderUnderLayer();

	int trg = ApplicationBase::GetInstance()->GetTrg();

	switch(_state)
	{
		// FADE_IN を使わないため省略

	case ModeBase::State::WAIT:
	{
		if(trg & PAD_INPUT_2)
		{
			// 修正: _handle のチェックを先に行う
			if(_handle != -1)
			{
				MovieManager::GetInstance()->StopMovie(_handle);
				MovieManager::GetInstance()->UnloadMovie(_handle);

				if(gGlobal._soundServer)
				{
					auto bgm = gGlobal._soundServer->Get("74");
					if(bgm)
					{
						bgm->Stop();
					}
				}
				_handle = -1;
			}

			_state = ModeBase::State::FADE_OUT;
			Fade::GetInstance()->FadeOut(0, 0, 0, FADE_FRAME);	// フェードアウト開始
			break;
		}

		// 再生終了を監視し、終わったらフェードアウトへ
		if(_handle != -1)
		{
			if(!MovieManager::GetInstance()->IsMoviePlaying(_handle))
			{
				if(gGlobal._soundServer)
				{
					auto bgm = gGlobal._soundServer->Get("74");
					if(bgm)
					{
						bgm->Stop();
					}
				}
				_state = ModeBase::State::FADE_OUT;
				Fade::GetInstance()->FadeOut(0, 0, 0, FADE_FRAME);	// フェードアウト開始
			}
		}
		else
		{
			// ハンドル無ければ即フェードアウト
			_state = ModeBase::State::FADE_OUT;
			Fade::GetInstance()->FadeOut(0, 0, 0, FADE_FRAME);
		}
		break;
	}
	case ModeBase::State::FADE_OUT:
	{
		Fade::GetInstance()->Process();
		// 修正: UnloadMovie 後に _handle を -1 にリセットして毎フレーム呼ばれるのを防ぐ
		if(_handle != -1)
		{
			MovieManager::GetInstance()->UnloadMovie(_handle);
			_handle = -1;
		}
		if(Fade::GetInstance()->IsFade() == false)
		{
			_state = ModeBase::State::DONE;
		}
		break;
	}
	case ModeBase::State::DONE:
	{
		Terminate();
		ModeServer::GetInstance()->Add(new ModeInit(), 0, "init");
		ModeServer::GetInstance()->Del(this);

		break;
	}
	}

	return true;
}

bool ModeCredit::Render()
{
	// 再生中のムービーを描画（Play は Initialize で一度だけ行っている）
	if(_handle != -1)
	{
		DrawGraph(0, 0, _handle, TRUE);
	}

	if(_state == ModeBase::State::WAIT)
	{
		DrawGraph(credit::CREDIT_X, credit::CREDIT_Y, _creditHandle, TRUE);
	}

	// フェードを重ねる（フェードアウト時に有効）
	Fade::GetInstance()->Render();
	return true;
}