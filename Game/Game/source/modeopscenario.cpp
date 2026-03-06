#include "modeopscenario.h"
#include "modegame.h"
#include "modegameload.h"

#ifdef _DEBUG
#include <crtdbg.h>
#define NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
#else
#define NEW new
#endif

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
	base::Initialize();
	_soundServer = std::make_shared<soundserver::SoundServer>();

	_backHandle = LoadGraph("res/Prologue/BG_opstory.png");	// 背景画像の読み込み

	_page =
	{
		{ LoadGraph("res/Prologue/1.png"), ""},// 画像読み込み, 音声ファイル
		{ LoadGraph("res/Prologue/2.png"), ""},// 画像読み込み, 音声ファイル
		{ LoadGraph("res/Prologue/3.png"), ""},// 画像読み込み, 音声ファイル
		{ LoadGraph("res/Prologue/4.png"), ""},// 画像読み込み, 音声ファイル
		{ LoadGraph("res/Prologue/5.png"), ""},// 画像読み込み, 音声ファイル
		{ LoadGraph("res/Prologue/6.png"), ""},// 画像読み込み, 音声ファイル
		{ LoadGraph("res/Prologue/7.png"), ""},// 画像読み込み, 音声ファイル
		{ LoadGraph("res/Prologue/8.png"), ""},// 画像読み込み, 音声ファイル
		{ LoadGraph("res/Prologue/9.png"), ""},// 画像読み込み, 音声ファイル
		{ LoadGraph("res/Prologue/10.png"), ""},// 画像読み込み, 音声ファイル
	};

	// ページの表示データを設定
	_panelData =
	{
		{ 50,  50,  250, 250, 0.0f },   // ページ1の位置・サイズ・透明度
		{ 50, 200,  250, 250, 0.0f },   // ページ2
		{ 50,  450, 250, 250, 0.0f },   // ページ3
		{ 300,  50, 250, 250, 0.0f },   // ページ4
		{ 300, 300, 250, 250, 0.0f },   // ページ5
		{ 300, 500, 250, 250, 0.0f },   // ページ6
		{ 550,  50, 300, 300, 0.0f },   // ページ7
		{ 550, 340, 350, 350, 0.0f },   // ページ8
		{ 900,  50, 400, 400, 0.0f },   // ページ9
		{ 920, 450, 300, 300, 0.0f },   // ページ10
	};
	_pageNo = 0;
	_fadeTimer = 0;

	if(!_page.empty())
	{
		_voice = std::make_shared<soundserver::SoundItemVoice>(_page[_pageNo].voiceFile);
		_soundServer->Add("voice",_voice);
		_voice->Play();
	}

	return true;
}

bool ModeOpScenario::Terminate()
{
	base::Terminate();

	return true;
}

bool ModeOpScenario::Process()
{
	base::Process();

	ModeServer::GetInstance()->SkipProcessUnderLayer();
	ModeServer::GetInstance()->SkipRenderUnderLayer();

	// サウンド更新
	_soundServer->Update();

	// フェードイン処理
	if(_pageNo >= 0 && _pageNo < StCas<int>(_panelData.size()))
	{
		if(_panelData[_pageNo].alpha < 1.0f)
		{
			_fadeTimer++;
			_panelData[_pageNo].alpha = StCas<float>(_fadeTimer) / FADE_FRAME;
			// 透明度が1.0fを超えないようにする	
			if(_panelData[_pageNo].alpha > 1.0f)
			{
				_panelData[_pageNo].alpha = 1.0f;
			}
		}
	}
	int trg = ApplicationBase::GetInstance()->GetTrg();

	switch(_state)
	{
		case ModeBase::State::WAIT:
		{
			if(trg & PAD_INPUT_2)
			{
				if (_pageNo >= 0 && _pageNo < StCas<int>(_panelData.size()))
				{
					_panelData[_pageNo].alpha = 1.0f;
				}

				// 次のページがあるなら進める
				if(_pageNo < StCas<int>(_page.size()) - 1)
				{
					_pageNo++;
					_fadeTimer = 0;

					// 音声があれば再生
					if(!_page[_pageNo].voiceFile.empty())
					{
						if (_soundServer)
						{
							_soundServer->StopType(soundserver::SoundItemBase::TYPE::VOICE);
							_voice = std::make_shared<soundserver::SoundItemVoice>(_page[_pageNo].voiceFile);
							_soundServer->Add("voice", _voice);
							_voice->Play();
						}
					}
				}
				else
				{
					// 最終ページなのでフェードアウト開始
					_state = ModeBase::State::FADE_OUT;
					Fade::GetInstance()->FadeOut(0, 0, 0, FADE_FRAME);	// フェードアウト開始
				}
			}
			if(trg & PAD_INPUT_1)
			{
				ModeServer::GetInstance()->Add(NEW ModeGame(), 255, "ModeGame");
				ModeServer::GetInstance()->Del(this);
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
			ModeServer::GetInstance()->Add(NEW ModeGame(), 255, "ModeGame");
			ModeServer::GetInstance()->Del(this);
			break;
		}
	}
	return true;
}

bool ModeOpScenario::Render()
{
	base::Render();

	// 背景の描画
	if(_backHandle != -1)
	{
		DrawGraph(0, 0, _backHandle, TRUE);
	}

	// 表示されているコマを順番に描画
	for(int i = 0; i <= _pageNo && i < StCas<int>(_page.size()); i++)
	{
		if(_page[i].handle != -1 && i < StCas<int>(_panelData.size()))
		{
			// コマを明るくするエフェクトを追加
			SetDrawBlendMode(DX_BLENDMODE_ADD, 30); // 加算ブレンドで少し明るく
			// 通常の描画
			SetDrawBlendMode(DX_BLENDMODE_ALPHA, StCas<int>(255 * _panelData[i].alpha));
			DrawExtendGraph
			(
				_panelData[i].x,
				_panelData[i].y,
				_panelData[i].x + _panelData[i].w,
				_panelData[i].y + _panelData[i].h,
				_page[i].handle,
				TRUE
			);
			SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
		}
	}
	Fade::GetInstance()->Render();	// フェード描画
	return true;
}
