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
	// _handleLoadingBG = LoadGraph("res/ui/loading_bg.png");
	// _handleLoadingBar = LoadGraph("res/ui/loading_bar.png");
	// _handlePressButton = LoadGraph("res/ui/press_button.png");

	// フェードイン開始
	Fade::GetInstance()->ColorMask(0, 0, 0, 255);
	Fade::GetInstance()->FadeIn(FADE_FRAME);

	_state = ModeBase::State::FADE_IN;
	_fadeTimer = 0;

	// ApplicationGlobalの初期化を開始（非同期読み込み開始）
	gGlobal.Init();

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
		// ボタン点滅アニメーション
		_buttonAlpha = std::abs(std::sin(GetModeTm() / 500.0f));

		// PAD_INPUT_2 で次へ
		if(_canProceed && (trg & PAD_INPUT_2))
		{
			_state = ModeBase::State::FADE_OUT;
			Fade::GetInstance()->FadeOut(0, 0, 0, FADE_FRAME);
		}
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
	else
	{
		// デフォルト背景（黒）
		DrawBox(0, 0, 1920, 1080, GetColor(0, 0, 0), TRUE);
	}

	// ローディングテキスト + ドットアニメーション
	std::string loadingText = "Loading";
	for(int i = 0; i < _dotCount; i++)
	{
		loadingText += ".";
	}
	DrawString(860, 500, loadingText.c_str(), GetColor(255, 255, 255));

	// プログレスバー描画
	int barX = 760;
	int barY = 550;
	int barWidth = 400;
	int barHeight = 30;

	// 枠
	DrawBox(barX - 2, barY - 2, barX + barWidth + 2, barY + barHeight + 2, GetColor(255, 255, 255), FALSE);

	// 進捗バー（ApplicationGlobalから取得）
	int currentProgress = gGlobal.GetLoadProgress();
	int fillWidth = barWidth * currentProgress / 100;
	DrawBox(barX, barY, barX + fillWidth, barY + barHeight, GetColor(100, 200, 255), TRUE);

	// パーセント表示
	DrawFormatString(barX + barWidth / 2 - 20, barY + 5, GetColor(255, 255, 255), "%d%%", currentProgress);

	// 読み込み完了時のボタン表示
	if(_canProceed)
	{
		int alpha = static_cast<int>(_buttonAlpha * 255);
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);

		if(_handlePressButton != -1)
		{
			DrawGraph(760, 650, _handlePressButton, TRUE);
		}
		else
		{
			DrawString(800, 650, "Press Button to Continue", GetColor(255, 255, 255));
		}

		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	}

	// フェード描画
	Fade::GetInstance()->Render();

	return true;
}