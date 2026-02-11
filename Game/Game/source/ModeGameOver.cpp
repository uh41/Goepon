#include "ModeGameOver.h"
#include "ApplicationMain.h"
#include "modegame.h"

bool ModeGameOver::Initialize()
{
	if (!base::Initialize()) return false;
	return true;
}

bool ModeGameOver::Terminate()
{
	base::Terminate();
	return true;
}

bool ModeGameOver::Process()
{
	base::Process();
	// ゲームオーバー画面が出ている間は「下のレイヤー(ゲーム本編)」を止める（描画は止めない）
	ModeServer::GetInstance()->SkipProcessUnderLayer();
	// キー取得
	int trg = ApplicationBase::GetInstance()->GetTrg();

	//// パッド1のボタンが押されたらモード削除
	if(trg & PAD_INPUT_1)
	{
		if(_ownerGame)
		{
			if(auto* game = dynamic_cast<ModeGame*>(_ownerGame))
			{
				game->RequestResetStage();
			}
			// ゲームオーバー画面を閉じる
			ModeServer::GetInstance()->Del(this);
		}
	}
	return true;
}

bool ModeGameOver::Render()
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

	// ゲームオーバーメッセージ
	SetFontSize(TitleFontSize);
	DrawString
	(
		TitlePosX, TitlePosY,
		GameOverMessage,
		GetColor(WhiteR, WhiteG, WhiteB)
	);

	// ヒントメッセージ
	SetFontSize(HintFontSize);
	DrawString
	(
		HintPosX, HintPosY,
		HintMessage,
		GetColor(HintR, HintG, HintB)
	);

	return true;

}
