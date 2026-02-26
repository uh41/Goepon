/*********************************************************************/
// * \file   modebase.h
// * \brief  モードベースクラス
// *
// * \author 鈴木裕稀
// * \date   2025/12/15
// * \作業内容: 新規作成 鈴木裕稀　2025/12/15
/*********************************************************************/

#pragma once
#include "../container.h"
#include "../gameconfig.h"

class ModeServer;

//namespace fade
//{
//	static constexpr auto FADE_FRAME = 60;
//	static constexpr auto FADE_WAIT = 60;
//	static constexpr auto FADE_OUT_WAIT = 30;
//}


namespace
{
	// UI定数
	constexpr int BackgroundAlpha = 128; // 背景の透明度

	// 表示領域
	constexpr int sw = 1280;
	constexpr int sh = 720;

	// 幅
	constexpr int boxW = 820;
	constexpr int boxH = 240;

	// 背景矩形
	constexpr int BgLeft = (sw - boxW) / 2;
	constexpr int BgTop = (sh - boxH) / 2;
	constexpr int BgRight = BgLeft + boxW;
	constexpr int BgBottom = BgTop + boxH;

	// 文字の位置
	constexpr int TitlePosX = 500;
	constexpr int TitlePosY = BgTop + 40;
	constexpr int HintPosX = 500;
	constexpr int HintPosY = 400;

	// フォント
	constexpr int TitleFontSize = 50;
	constexpr int HintFontSize = 28;

	// 色
	constexpr int BlackR = 0;
	constexpr int BlackG = 0;
	constexpr int BlackB = 0;

	constexpr int WhiteR = 255;
	constexpr int WhiteG = 255;
	constexpr int WhiteB = 255;

	constexpr int HintR = 200;
	constexpr int HintG = 200;
	constexpr int HintB = 200;

	// メッセージ関係
	//ゲームクリアのメッセージ
	constexpr const char* ClearMessage = "ゲームクリア";
	//constexpr const char* HintMessage = "決定ボタンで閉じる";

	// ゲームオーバーのメッセージ
	constexpr const char* GameOverMessage = "いただきます";
	constexpr const char* HintMessage = "決定ボタンで閉じる";
}


class		ModeBase
{
public:
	enum class State
	{
		FADE_IN,
		WAIT,
		LOADING,
		LOADING_DONE,
		FADE_OUT,
		DONE
	};

	ModeBase();
	virtual ~ModeBase();

	virtual bool	Initialize();
	virtual bool	Terminate();
	virtual bool	Process();
	virtual bool	Render();


public:
	int	GetModeCount() { return _cntMode; }			// このモードが始まってからのカウンタ
	unsigned long GetModeTm() { return _tmMode; }	// このモードが始まってからの時間ms
	unsigned long GetStepTm() { return _tmStep; }	// 前フレームからの経過時間ms

	void SetCallPerFrame(int frame) { _callPerFrame = _callPerFrameCnt = frame; }	// 何フレームに1回Process()を呼ぶか(def:1)
	void SetCallOfCount(int count) { _callOfCount = count; }		// 1回の呼び出しに何回Process()を呼ぶか(def:1)
	int GetCallPerFrame() { return _callPerFrame; }
	int GetCallOfCount() { return _callOfCount; }

private:
	friend	ModeServer;
	// ModeServer用
	std::string		_szName;
	int				_uid;
	int				_layer;

	void StepTime(unsigned long tmNow);
	void StepCount();

	// モード内処理用
	int		_cntMode;	// 本モードに入ってからのカウンタ, 0スタート
	unsigned long	_tmMode;	// 本モードに入ってからの時間。ms
	unsigned long	_tmStep;	// 前フレームからの経過時間。ms
	unsigned long	_tmModeBase;	// このモードが始まった時間。ms
	unsigned long	_tmPauseBase;	// ポーズ中のベース時間。ms
	unsigned long	_tmPauseStep;	// ポーズ中の積算時間。ms 動作中、この値を _tmMode に用いる
	unsigned long	_tmOldFrame;	// 前フレームの時間。ms

	// CallPerFrame / CallOfCount用
	int		_callPerFrame;
	int		_callPerFrameCnt;
	int		_callOfCount;

protected:
	int _handle;

	int _loadProgress;	// ローディング進行度（0-100）

	State _state;

	int _fadeTimer;
	int _waitTimer;

	bool _isWait;

};


