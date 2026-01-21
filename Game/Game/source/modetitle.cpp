#include "modetitle.h"

// ModeBase / Fade は appframe.h 経由で参照可能な前提
ModeTitle::ModeTitle()
{
	Initialize();
}

ModeTitle::~ModeTitle()
{
	Terminate();
}

bool ModeTitle::Initialize()
{
	// 背景画像を読み込む（パスはプロジェクトのリソース配置に合わせて変更）
	_iHandle = LoadGraph("res/logo/AMGlogo.png");
	if(_iHandle == -1)
	{
		// 読み込み失敗時は false を返して呼び出し元で判定できるようにする
		return false;
	}

	// フェードの初期化（黒→透過でフェードイン）
	Fade::GetInstance()->ColorMask(0, 0, 0, 255);
	Fade::GetInstance()->FadeIn(FADE_FRAME);

	_state = ModeBase::State::FADE_IN;
	_fadeTimer = 0;

	return true;
}

bool ModeTitle::Terminate()
{
	if(_iHandle != -1)
	{
		DeleteGraph(_iHandle);
		_iHandle = -1;
	}
	return true;
}

bool ModeTitle::Process()
{
	// フェード進行
	Fade::GetInstance()->Process();

	// タイトルは最上位レイヤーとして下位の処理と描画をスキップ
	ModeServer::GetInstance()->SkipProcessUnderLayer();
	ModeServer::GetInstance()->SkipRenderUnderLayer();

	switch(_state)
	{
	case ModeBase::State::FADE_IN:
		if(Fade::GetInstance()->IsFade() == false)
		{
			_state = ModeBase::State::WAIT;
			_fadeTimer = FADE_WAIT;
		}
		break;
	case ModeBase::State::WAIT:
		// カウントダウン等の処理
		_fadeTimer--;
		if(_fadeTimer <= 0)
		{
			// 何もしないか、自動で次に遷移したければここでフェードアウトを開始
		}
		break;
	case ModeBase::State::FADE_OUT:
		if(Fade::GetInstance()->IsFade() == false)
		{
			// 終了処理（他モードへ移行する場合は Add を呼ぶ）
			ModeServer::GetInstance()->Del(this);
			_state = ModeBase::State::DONE;
		}
		break;
	case ModeBase::State::DONE:
	default:
		break;
	}

	return true;
}

bool ModeTitle::Render()
{
	if(_iHandle != -1)
	{
		DrawGraph(0, 0, _iHandle, TRUE);
	}

	// フェード描画（上書き）
	Fade::GetInstance()->Render();

	return true;
}