#include "modetitle.h"
#include "modeopscenario.h"

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
	_handle = LoadGraph(img::title);
	if(_handle == -1)
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
	if(_handle != -1)
	{
		DeleteGraph(_handle);
		_handle = -1;
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

	int trg = ApplicationBase::GetInstance()->GetTrg();

	switch(_state)
	{
	case ModeBase::State::FADE_IN:
		if(Fade::GetInstance()->IsFade() == false)
		{
			_state = ModeBase::State::WAIT;
		}
		break;
	case ModeBase::State::WAIT:
		if(trg & PAD_INPUT_2)
		{
			_state = ModeBase::State::FADE_OUT;
			Fade::GetInstance()->FadeOut(0, 0, 0, FADE_FRAME);
		}
		break;
	case ModeBase::State::FADE_OUT:
		if(Fade::GetInstance()->IsFade() == false)
		{
			_state = ModeBase::State::DONE;
		}
		break;
	case ModeBase::State::DONE:
		
		// 次のモードへ移行
		ModeServer::GetInstance()->Add(new ModeOpScenario(), 1, "opscenario");
		ModeServer::GetInstance()->Del(this);
		break;
	}

	return true;
}

bool ModeTitle::Render()
{
	if(_handle != -1)
	{
		DrawGraph(0, 0, _handle, TRUE);
	}

	// フェード描画（上書き）
	Fade::GetInstance()->Render();

	return true;
}