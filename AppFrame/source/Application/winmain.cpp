/*********************************************************************/
// * \file   winmain.cpp
// * \brief  DXライブラリ用 WinMain
// *
// * \author 鈴木裕稀
// * \date   2025/12/15
// * \作業内容: 新規作成 鈴木裕稀　2025/12/15
/*********************************************************************/

/*
** WinMain
*/

//
// include 部
//

#include "../appframe.h"
//#include <pybind11/pybind11.h>
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#include <chrono>
#include <thread>

//
// WinMain(). プログラム起動関数
//
int WINAPI WinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nCmdShow
) {
	SetOutApplicationLogValidFlag(false);
	// メモリリークチェック開始
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	//_CrtSetBreakAlloc(101700);

	ApplicationBase *appBase = ApplicationBase::GetInstance();
	if(!appBase) { return 0; }

	if(!appBase->Initialize(hInstance))
	{
		return 0;
	}

	// フレームレート制御インスタンスを取得
	auto frameRateController = appBase->GetFrameRateController();

	// 1フレームループを組む ----------------------------------------------------------
	while (ProcessMessage() == 0 && !appBase->IsExitRequested())		// プログラムが終了するまでループ
	{
		frameRateController->BeginFrame();

		appBase->Input();
		appBase->Process();

		ClearDrawScreen();		// 画面を初期化する
		appBase->Render();
		frameRateController->EndFrame();
		ScreenFlip();			// 裏画面の内容を表画面に反映させる
	}

	appBase->Terminate();

	return 0;
}
