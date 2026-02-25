#pragma once
#include "container.h"
class FrameRateController
{
public:
	FrameRateController(double targetFps = 60.0);
	~FrameRateController();

	void Initialize();					// 初期化
	void BeginFrame();					// フレーム開始
	void EndFrame();					// フレーム終了
	void SetTargetFps(double fps);		// 目標FPSの設定
	int GetCurrentFps() const;			// 現在のFPSを取得(ながくなるためcppに中身を作成)
	double GetDeltaTime() const;		// デルタタイムを取得
	void EnableFpsDisplay(bool enable);	// FPS表示の有効化/無効化
	void ResetFrameCounter();			// フレームカウンタのリセット

private:
	double _targetFps;
	std::chrono::duration<double> _frameDuration;			  // 1フレームの理想的な時間
	std::chrono::steady_clock::time_point _nextFrameTime;	  // 次のフレーム開始予定時刻
	std::chrono::steady_clock::time_point _lastFrameTime;	  // 最後のフレーム開始時刻
	std::chrono::steady_clock::time_point _fpsLastUpdateTime; // 最後にFPSを更新した時刻

	int _frameCount;       // フレームカウント
	int _currentFps;       // 現在のFPS
	double _deltaTime;     // デルタタイム（秒）
	bool _initialized;     // 初期化フラグ
	bool _showFpsDisplay;  // FPS表示フラグ

	void UpdateFpsCounter(); // FPSカウンタの更新
	void WaitForNextFrame(); // 次のフレーム開始まで待機
	void RenderFpsDisplay(); // FPS表示の描画
};