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

	// パッド1のボタンが押されたらモード削除
	if(trg & PAD_INPUT_1)
	{
		ModeServer::GetInstance()->Del(this);
		ModeServer::GetInstance()->Add(new ModeGame(), 0, "ModeTeamLogo");
	}
	return true;
}

bool ModeGameClear::Render()
{
	base::Render();

	// 半透明の背景
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 128);
	DrawBox(200, 150, 600, 350, GetColor(0, 0, 0), TRUE);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

	// クリアメッセージ
	DrawString(260, 200, "ゲームクリア", GetColor(255, 255, 255));
	SetFontSize(24);
	DrawString(260, 260, "決定ボタンで閉じる", GetColor(200, 200, 200));
	return true;
}
