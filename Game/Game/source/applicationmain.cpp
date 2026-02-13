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
#include "modeteamlogo.h"
#include "modeopscenario.h"

#ifdef _DEBUG
#include <crtdbg.h>
#define NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
#else
#define NEW new
#endif

// 実体
ApplicationMain g_application_main;

// DXLib_Init()前の処理
bool ApplicationMain::BeforeDXLib_Init()
{
	// 3Dsound:XAudioを使用する
	SetEnableXAudioFlag(TRUE);

	// 3DSound:1メートルに相当する値を設定
	Set3DSoundOneMetre(1.0f);

	return true;
}

// 初期化
bool ApplicationMain::Initialize(HINSTANCE hInstance)
{
	if(!base::Initialize(hInstance)) { return false; }

	// アプリケーショングローバルの初期化
	gGlobal.Init();

	// モードの登録
	//ModeServer::GetInstance()->Add(new ModeInit(), 4, "logo");
	//ModeServer::GetInstance()->Add(new ModeTeamLogo(), 3, "teamlogo");
	//ModeServer::GetInstance()->Add(new ModeTitle(), 2, "title");
	//ModeServer::GetInstance()->Add(new ModeOpScenario(), 1, "opscenario");
	ModeServer::GetInstance()->Add(NEW ModeGame(), 0, "game");

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
