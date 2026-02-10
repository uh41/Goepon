#include "ModeGameClear.h"
#include "ApplicationMain.h"
#include "modegame.h"
#include "modeteamlogo.h"

bool ModeGameClear::Initialize()
{
	if(!base::Initialize()) return false;
	_fontSize = 48;
	return true;
}

bool ModeGameClear::Terminate()
{
	base::Terminate();
	return true;
}

bool ModeGameClear::Process()
{
	base::Process();
	// クリア画面が出ている間は「下のレイヤー(ゲーム本編)」を止める（描画は止めない）
	ModeServer::GetInstance()->SkipProcessUnderLayer();
	// キー取得
	int trg = ApplicationMain::GetInstance()->GetTrg();

	//// パッド1のボタンが押されたらモード削除
	if (trg & PAD_INPUT_1)
	{
		// 下層の ModeGame にリセット要求
		if(_ownerGame)
		{
			if(auto* game = dynamic_cast<ModeGame*>(_ownerGame))
			{
				game->RequestResetStage();
			}
			// クリア画面を閉じる
			ModeServer::GetInstance()->Del(this);
		}
	}
	return true;
}

bool ModeGameClear::Render()
{
	base::Render();

	// 背景(半透明に設定)
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, BackgroundAlpha);
	DrawBox
	(
		BgLeft, BgTop, BgRight, BgBottom,
		GetColor(BlackR, BlackG, BlackB),
		TRUE
	);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

	// 枠
	DrawBox
	(
		BgLeft, BgTop, BgRight, BgBottom,
		GetColor(255, 255, 255),
		FALSE
	);

	// クリアメッセージ
	SetFontSize(50);
	DrawString
	(
		TitlePosX, TitlePosY,
		"ゲームクリア",
		GetColor(WhiteR, WhiteG, WhiteB)
	);

	// ヒントメッセージ
	SetFontSize(HintFontSize);
	DrawString
	(
		HintPosX, HintPosY,
		"決定ボタンで閉じる",
		GetColor(HintR, HintG, HintB)
	);

	return true;
}
