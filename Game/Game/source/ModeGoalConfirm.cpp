/*********************************************************************/
// * \file   ModeGoalConfirm.cpp
// * \brief  ゲームクリア確認クラス
// *
// * \author 石森虹大
// * \date   2026/2/9
// * \作業内容	: 新規作成 石森虹大　2026/2/9
//				
/*********************************************************************/


#include "ModeGoalConfirm.h"
#include "applicationmain.h"

ModeGoalConfirm::ModeGoalConfirm(Result* ooutResult)
	: _outResult(ooutResult)
	, _cursor(1) // デフォルトは「帰らない」
	, _animCnt(0)
{

}


bool ModeGoalConfirm::Initialize()
{
	if(!base::Initialize()) { return false; }
	if(_outResult)
	{
		*_outResult = Result::None;
	}

	_cursor = 1; // デフォルトは「帰らない」
	_animCnt = 0;
	return true;
}

bool ModeGoalConfirm::Terminate()
{
	base::Terminate();
	return true;
}

bool ModeGoalConfirm::Process()
{
	base::Process();
	
	//下のレイヤは止める
	ModeServer::GetInstance()->SkipProcessUnderLayer();

	const int trg = ApplicationMain::GetInstance()->GetTrg();
	
	// 左右で選択
	if(trg & PAD_INPUT_LEFT)
	{
		_cursor = 0; // Yes
	}
	else if(trg & PAD_INPUT_RIGHT)
	{
		_cursor = 1; // No
	}

	// 決定
	if(trg & PAD_INPUT_1)
	{
		if(_outResult)
		{
			*_outResult = (_cursor == 0) ? Result::Yes : Result::No;
		}
		ModeServer::GetInstance()->Del(this); // 自分を削除
		return true;
	}
}

bool ModeGoalConfirm::Render()
{
	base::Render();

	// 表示領域
	const int sw = 1280;
	const int sh = 720;

	const int boxW = 820;
	const int boxH = 240;
	const int x0 = (sw - boxW) / 2;
	const int y0 = (sh - boxH) / 2;
	const int x1 = x0 + boxW;
	const int y1 = y0 + boxH;

	// 背景(半透明に設定)
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 128);
	DrawBox(x0, y0, x1, y1, GetColor(0, 0, 0), TRUE);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	
	// 枠
	DrawBox(x0, y0, x1, y1, GetColor(255, 255, 255), FALSE);

	// テキスト
	SetFontSize(32);
	DrawString(x0, y0 + 40, "ゴールしました。帰りますか？", GetColor(255, 255, 255));

	// 選択肢
	SetFontSize(28);
	const int itemY = y0 + 120;

	const char* yesText = "帰還する";
	const char* noText = "帰還しない";

	const int yesX = x0 + 170;
	const int noX  = x0 + 500;

	DrawString(yesX, itemY, yesText, GetColor(255, 255, 255));
	DrawString(noX, itemY, noText, GetColor(255, 255, 255));

	// カーソル（左右に移動）
	const int cursorBaseX = (_cursor == 0) ? (yesX - 40) : (noX - 40);
	const int cursorX = cursorBaseX + ((_animCnt / 4) % 16); // アニメーション

	DrawString(cursorX, itemY, "→", GetColor(255, 255, 0));

	// 操作説明
	SetFontSize(22);
	DrawString(x0 + 40, y1 - 45, "←/→:選択   A:決定", GetColor(200, 200, 200));

	return true;
}
