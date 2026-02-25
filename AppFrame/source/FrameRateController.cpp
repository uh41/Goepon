#include "FrameRateController.h"

FrameRateController::FrameRateController(double targetFps)
	: _targetFps(targetFps)
	, _frameDuration(std::chrono::duration<double>(1.0 / targetFps))  // 明示的にキャスト
	, _frameCount(0)
	, _currentFps(0)
	, _deltaTime(0.0)
	, _initialized(false)
	, _showFpsDisplay(true)
{
}

FrameRateController::~FrameRateController()
{
}

void FrameRateController::Initialize()
{
	auto now = std::chrono::steady_clock::now();
	_nextFrameTime = now + std::chrono::duration_cast<std::chrono::steady_clock::duration>(_frameDuration);
	_lastFrameTime = now;
	_fpsLastUpdateTime = now;
	_frameCount = 0;
	_currentFps = 0;
	_deltaTime = 0.0;
	_initialized = true;
}

void FrameRateController::BeginFrame()
{
	if(!_initialized)
	{
		Initialize();
	}

	auto now = std::chrono::steady_clock::now();

	// デルタタイムの計算
	auto elapsed = now - _lastFrameTime;
	_deltaTime = std::chrono::duration<double>(elapsed).count();
	_lastFrameTime = now;
}

void FrameRateController::EndFrame()
{
	// FPS計測
	UpdateFpsCounter();

	// FPS表示
	if(_showFpsDisplay)
	{
		RenderFpsDisplay();
	}

	// フレームレート制御
	WaitForNextFrame();
}

void FrameRateController::SetTargetFps(double fps)
{
	_targetFps = fps;
	_frameDuration = std::chrono::duration<double>(1.0 / fps);
}

int FrameRateController::GetCurrentFps() const
{
	return _currentFps;
}

double FrameRateController::GetDeltaTime() const
{
	return _deltaTime;
}

void FrameRateController::EnableFpsDisplay(bool enable)
{
	_showFpsDisplay = enable;
}

void FrameRateController::ResetFrameCounter()
{
	auto now = std::chrono::steady_clock::now();
	_fpsLastUpdateTime = now;
	_frameCount = 0;
	_currentFps = 0;
}

void FrameRateController::UpdateFpsCounter()
{
	++_frameCount;

	auto now = std::chrono::steady_clock::now();
	auto elapsed = now - _fpsLastUpdateTime; // 前回のFPS更新からの経過時間

	// 1秒経過したらFPSを更新
	if(elapsed >= std::chrono::seconds(1)) {
		_currentFps = _frameCount;
		_frameCount = 0;
		_fpsLastUpdateTime = now;
	}
}

void FrameRateController::WaitForNextFrame()
{
	auto now = std::chrono::steady_clock::now();

	if(now < _nextFrameTime) 
	{
		auto remain = _nextFrameTime - now;

		// 十分に時間があるならスリープ
		if(remain > std::chrono::milliseconds(2)) 
		{
			std::this_thread::sleep_for(remain - std::chrono::milliseconds(1)); // 少し余裕を持たせてスリープ
		}

		// 精密な待機（スピンウェイト）
		while(std::chrono::steady_clock::now() < _nextFrameTime) 
		{
			// 短時間のスピンウェイト
		}
	}
	else
	{
		// 処理が遅れている場合は現在時刻に追従（ドリフト防止）
		_nextFrameTime = std::chrono::steady_clock::now();
	}

	// 次フレーム目標時刻を更新
	_nextFrameTime += std::chrono::duration_cast<std::chrono::steady_clock::duration>(_frameDuration);
}

void FrameRateController::RenderFpsDisplay()
{
	DrawFormatString(0, 0, GetColor(255, 0, 0), "FPS: %d", _currentFps);
}