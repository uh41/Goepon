#pragma once
#include "appframe.h"

namespace
{
	// ゲームオーバー UI定数
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

	// メッセージ
	constexpr const char* GameOverMessage = "いただきます";
	constexpr const char* HintMessage = "決定ボタンで閉じる";
}


class ModeGameOver :public ModeBase
{
	typedef ModeBase base;
public:
	explicit ModeGameOver(ModeBase* ownerGame = nullptr) : _ownerGame(ownerGame) {} // コンストラクタ

	virtual bool Initialize() override;
	virtual bool Terminate() override;
	virtual bool Process() override;
	virtual bool Render() override;

private:
	ModeBase* _ownerGame; // 所有しているゲームモードへのポインタ
};

