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
#include <pybind11/pybind11.h>
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
	ApplicationBase *appBase = ApplicationBase::GetInstance();
	if(!appBase) { return 0; }

	if(!appBase->Initialize(hInstance))
	{
		return 0;
	}

	// 60fpsに設定
	constexpr double targetFps = 60.0;
	// １秒の長さ
	const auto frameDuration = std::chrono::duration<double>(1.0 / targetFps);

	// 初回基準時刻
	auto nextFrameTime = std::chrono::steady_clock::now() + frameDuration;
	
	// FPS計測用
	auto fpsLastTime   = std::chrono::steady_clock::now(); // 最後にFPSを更新した時刻
	auto fpsFrameCount = 0; // フレームカウント
	auto currentFps    = 0; // 現在のFPS
	// 1フレームループを組む ----------------------------------------------------------
	while (ProcessMessage() == 0)		// プログラムが終了するまでループ
	{
		appBase->Input();
		appBase->Process();

		ClearDrawScreen();		// 画面を初期化する
		appBase->Render();

		++fpsFrameCount;
		auto fpsNow = std::chrono::steady_clock::now();
		if(fpsNow - fpsLastTime >= std::chrono::seconds(1))
		{
			currentFps = fpsFrameCount;
			fpsFrameCount = 0;
			// 厳密な一秒周期にするなら,fpsLastTime += 1s でも良いが、now に合わせてリセットする
			fpsLastTime = fpsNow;
		}
		// デバック用　(FPS表示)
		DrawFormatString(0, 0, GetColor(255, 255, 255), "FPS: %d", currentFps);

		ScreenFlip();			// 裏画面の内容を表画面に反映させる

		// フレームレート制御
		auto now = std::chrono::steady_clock::now();
		if(now < nextFrameTime)
		{
			auto remain = nextFrameTime - now; // 次のフレームまでの残り時間
			// 次のフレーム時間を更新
			if(remain > std::chrono::milliseconds(2))
			{
				// 十分に時間があるならスリープ
				std::this_thread::sleep_for(remain - std::chrono::milliseconds(1));
			}
			// 待ち合わせ(スピン)
			while (std::chrono::steady_clock::now() < nextFrameTime) { /* short spin-wait */ }
		}
		else
		{
			// 処理が遅れている場合は現在時刻に追従してリセット（ドリフト防止）
			nextFrameTime = std::chrono::steady_clock::now();
		}

		// 次フレーム目標を進める（固定間隔）
		nextFrameTime += frameDuration;
		
	}

	appBase->Terminate();

	return 0;
}
