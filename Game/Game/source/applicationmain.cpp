/*********************************************************************/
// * \file   applicationmain.cpp
// * \brief  アプリケーションメインクラス
// *
// * \author 鈴木裕稀
// * \date   2025/12/15
// * \作業内容: 新規作成 鈴木裕稀　2025/12/15
/*********************************************************************/

#include "applicationmain.h"
#include "appframe.h"
#include "applicationglobal.h"
#include "modegame.h"
#include "modeinit.h"
#include "modetitle.h"

// 実体
ApplicationMain g_application_main;

// 初期化
bool ApplicationMain::Initialize(HINSTANCE hInstance)
{
	if(!base::Initialize(hInstance)) { return false; }

	// アプリケーショングローバルの初期化
	gGlobal.Init();

	// モードの登録
	ModeServer::GetInstance()->Add(new ModeInit(), 2, "logo");
	ModeServer::GetInstance()->Add(new ModeTitle(), 1, "title");
	ModeServer::GetInstance()->Add(new ModeGame(), 0, "game");

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
