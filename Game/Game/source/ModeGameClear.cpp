#include "ModeGameClear.h"
#include "ApplicationMain.h"

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
	// 下のレイヤーを動かさない(メニューと同様)
	ModeServer::GetInstance()->SkipProcessUnderLayer();

	// キー取得
	int key = ApplicationMain::GetInstance()->GetKey();
	int trg = ApplicationMain::GetInstance()->GetTrg();

	// パッド1のボタンが押されたらモード削除
	if(trg & PAD_INPUT_1)
	{
		ModeServer::GetInstance()->Del(this);
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
