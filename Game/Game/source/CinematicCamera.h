#pragma once
#include "camera.h"
class CinematicCamera : public Camera
{
	typedef Camera base;
public:
	CinematicCamera();
	virtual ~CinematicCamera() = default;

	virtual bool Initialize() override;
	virtual bool Terminate() override;
	virtual bool Process() override;
	virtual bool Render() override;

	// 演出の種類
	enum class State
	{
		Idle,
		Orbit,
		Zoom,
		Rotate,
		Shake,
	};

	// イージング関数の型定義
	using EasingFunc = float(*)(float cnt, float start, float end, float frames);
	// ターゲットを中心に、指定した半径で円軌道を描いて回る（ターゲットは固定）
	void StartOrbit(const vec::Vec3& target, float durationSeconds, float startRadius, float endRadius, float revolutions = 1.0f);
	// ターゲットに向かって、指定した距離を移動する（ターゲットは固定）
	void StartZoom(const vec::Vec3& target, float durationSeconds, float startDist, float endDist, EasingFunc easing = nullptr);
	//　ターゲットを中心に、指定した半径で円軌道を描いて回る（ターゲットは固定）。回転速度は一定。
	void StartRotateSpeed(float radiansPerSec, float durationSeconds); // 回転速度（ラジアン/秒）と継続時間を指定
	// 揺れの強さと継続時間を指定
	void StartShake(float intensity, float durationSeconds);

	// 演出をすべて停止する
	void StopAll();

	auto GetState() const { return _state; }

protected:

	State _state = State::Idle; // 現在の演出状態
	float _timer = 0.0f;        // 演出開始からの経過時間
	float _duration = 0.0f;     // 演出の継続時間

	//　共通パラメータ
	vec::Vec3 _targetPos; // 演出の対象

	// Orbit用パラメータ
	float _orbitStartRadius = 0.0f; // ターゲットからの距離の開始値
	float _orbitEndRadius = 0.0f;   // ターゲットからの距離の終了値
	float _orbitRevolutions = 0.0f; // 演出全体での回転数（例：1.0なら1周、0.5なら半周）
	float _orbitStartAngle = 0.0f;  // 演出開始時のカメラ位置から見たターゲットの角度（ラジアン）

	// Zoom用パラメータ
	float _zoomStartDist = 0.0f;	  // ターゲットからの距離の開始値
	float _zoomEndDist = 0.0f;		  // ターゲットからの距離の終了値
	EasingFunc _zoomEasing = nullptr; // ズームのイージング関数

	// Rotate用パラメータ
	float _rotateSpeed = 0.0f; // 回転速度（ラジアン/秒）

	// Shake用パラメータ
	float _shakeIntensity = 0.0f;		// 揺れの強さ
	float _shakeCyclesPerSecond = 8.0f; // 揺れの周期（1秒あたりの揺れの回数）
	vec::Vec3 _shakeBasePos{};			// 揺れの基準位置（演出開始時のカメラ位置）
private:
	static constexpr int kDebugTrailMax = 180; // デバック用

	bool _debugDrawEnabled = true;	         // デバッグ描画の有効フラグ
	int _debugTrailCount = 0;                // デバッグ用の軌跡のカウンタ
	int _debugTrailHead = 0;		         // デバッグ用の軌跡のヘッドインデックス
	vec::Vec3 _debugTrail[kDebugTrailMax]{}; // デバッグ用の軌跡の位置配列
};
