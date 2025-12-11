#include "applicationmain.h"
#include "appframe.h"
#include "applicationglobal.h"
#include "modegame.h"

// 実体
ApplicationMain g_application_main;

// 初期化
bool ApplicationMain::Initialize(HINSTANCE hInstance)
{
	if(!base::Initialize(hInstance)) { return false; }

	// アプリケーショングローバルの初期化
	gGlobal.Init();

	// モードの登録
	ModeServer::GetInstance()->Add(new ModeGame(), 1, "game");

	return true;
}

// 終了
bool ApplicationMain::Terminate()
{
	base::Terminate();
	return true;
}

// 入力処理
bool ApplicationMain::Input()
{
	base::Input();
	return true;
}

// 計算処理
bool ApplicationMain::Process()
{
	base::Process();
	return true;
}

// 描画処理
bool ApplicationMain::Render()
{
	base::Render();
	return true;
}
