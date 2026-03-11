/*********************************************************************/
// * \file   modeloading.cpp
// * \brief  ロード画面モード
// *
// * \author 鈴木裕稀
// * \date   2026/02/19
// * \作業内容: 新規作成 鈴木裕稀　2026/02/19
/*********************************************************************/
#include "modeloading.h"
#include "modeopscenario.h"
#include "applicationglobal.h"
#include <cmath>

#ifdef _DEBUG
#include <crtdbg.h>
#define NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
#else
#define NEW new
#endif

ModeLoading::ModeLoading()
	: _handleLoadingBG(-1)
	, _handleLoadingBar(-1)
	, _handlePressButton(-1)
	, _dotAnimTime(0.0f)
	, _dotCount(0)
	, _canProceed(false)
	, _buttonAlpha(0.0f)
{
	Initialize();
}

ModeLoading::~ModeLoading()
{
	Terminate();
}

bool ModeLoading::Initialize()
{
	// ローディング画面のリソース読み込み（これは同期で即座に読む）
	// TODO: 実際の画像パスに置き換えてください
	_handleLoadingBG = LoadGraph(img::Loading_1start);
	// _handleLoadingBar = LoadGraph("res/ui/loading_bar.png");
	// _handlePressButton = LoadGraph("res/ui/press_button.png");

	// フェードイン開始
	Fade::GetInstance()->ColorMask(0, 0, 0, 255);
	Fade::GetInstance()->FadeIn(FADE_FRAME);

	_state = ModeBase::State::FADE_IN;
	_fadeTimer = 0;

	// ApplicationGlobalの初期化を開始（非同期読み込み開始）
	//gGlobal.Init();

	return true;
}

bool ModeLoading::Terminate()
{
	// リソース解放
	if(_handleLoadingBG != -1)
	{
		DeleteGraph(_handleLoadingBG);
		_handleLoadingBG = -1;
	}
	if(_handleLoadingBar != -1)
	{
		DeleteGraph(_handleLoadingBar);
		_handleLoadingBar = -1;
	}
	if(_handlePressButton != -1)
	{
		DeleteGraph(_handlePressButton);
		_handlePressButton = -1;
	}

	return true;
}

bool ModeLoading::Process()
{
	// 下位レイヤーをスキップ
	ModeServer::GetInstance()->SkipProcessUnderLayer();
	ModeServer::GetInstance()->SkipRenderUnderLayer();

	int trg = ApplicationBase::GetInstance()->GetTrg();

	switch(_state)
	{
	case ModeBase::State::FADE_IN:
		Fade::GetInstance()->Process();
		if(Fade::GetInstance()->IsFade() == false)
		{
			_state = ModeBase::State::LOADING;
		}
		break;

	case ModeBase::State::LOADING:
		// ドットアニメーション更新
		_dotAnimTime += 0.1f;
		if(_dotAnimTime >= 1.0f)
		{
			_dotAnimTime = 0.0f;
			_dotCount = (_dotCount + 1) % 4; // 0, 1, 2, 3
		}

		// ApplicationGlobalのロード進行度を更新
		gGlobal.UpdateLoadProgress();

		// 読み込み完了チェック
		if(!gGlobal.IsLoading())
		{
			_state = ModeBase::State::LOADING_DONE;
			_canProceed = true;
		}
		break;

	case ModeBase::State::LOADING_DONE:
		// 読み込み完了後、自動的にフェードアウト
		_state = ModeBase::State::FADE_OUT;
		Fade::GetInstance()->FadeOut(0, 0, 0, FADE_FRAME);
		break;

	case ModeBase::State::FADE_OUT:
		Fade::GetInstance()->Process();
		if(Fade::GetInstance()->IsFade() == false)
		{
			_state = ModeBase::State::DONE;
		}
		break;

	case ModeBase::State::DONE:
		// シナリオモードへ遷移
		ModeServer::GetInstance()->Add(NEW ModeOpScenario(), 1, "opscenario");
		ModeServer::GetInstance()->Del(this);
		break;
	}

	return true;
}

bool ModeLoading::Render()
{
	// 背景描画
	if(_handleLoadingBG != -1)
	{
		DrawGraph(0, 0, _handleLoadingBG, TRUE);
	}

	// フェード描画
	Fade::GetInstance()->Render();

	return true;
}