#include "CinematicCamera.h"
#include "appframe.h"

CinematicCamera::CinematicCamera()
{
	_state = State::Idle;
	_timer = 0.0f;
	_duration = 0.0f;
	_effectTarget = vec::Vec3{ 0.0f, 0.0f, 0.0f };
	_orbitStartRadius = _orbitEndRadius = _orbitRevolutions = _orbitStartAngle = 0.0f;
	_zoomStartDist = _zoomEndDist = 0.0f;
	_rotateSpeed = 0.0f;
	_shakeIntensity = 0.0f;
}

bool CinematicCamera::Initialize()
{
	_state = State::Idle;
	_timer = 0.0f;
	_duration = 0.0f;
	return true;
}

bool CinematicCamera::Process()
{
	const float dt = 1.0f / 60.0f; // 仮のデルタタイム（60FPS想定）。実際にはアプリケーションのフレームタイムを使用するべき。

	if (_state == State::Idle)
	{
		// 演出なし
		return true;
	}
	_timer += dt;
	// mymath はフレームベースの引数なので秒->フレームへ変換
	const float fps = 1.0f / dt;
	const float frames = _duration * fps;
	const float cnt = _timer * fps;

	// 状態に応じた処理
	switch(_state)
	{
		case State::Orbit:
		{
			// 半径を mymath のイージングで補間
			float radius = mymath::EasingInOutQuad(cnt, _orbitStartRadius, _orbitEndRadius, frames);

			// 回転進行量もイージングしたい場合は revolutions をイージング（0->_orbitRevolutions）
			float easedRevs = mymath::EasingInOutQuad(cnt, 0.0f, _orbitRevolutions, frames);
			float angle = _orbitStartAngle + 2.0f * DX_PI_F * easedRevs;

			_vTarget = _effectTarget;
			_vPos.x = _effectTarget.x + cosf(angle) * radius;
			_vPos.z = _effectTarget.z + sinf(angle) * radius;
			_vPos.y = _effectTarget.y + 200.0f;
			break;
		}
		case State::Zoom:
		{
			// 距離を mymath で補間
			float dist = mymath::EasingInOutQuad(cnt, _zoomStartDist, _zoomEndDist, frames);

			// target からの方向ベクトル（現在位置を基準）
			vec::Vec3 dir = vec3::VSub(_vPos, _effectTarget);
			float len = vec3::VSize(dir);
			if (len > 1e-6f)
			{
				vec::Vec3 nd = vec3::VScale(dir, 1.0f / len);
				_vPos = vec3::VAdd(_effectTarget, vec::Vec3{ nd.x * dist, nd.y * dist, nd.z * dist });
			}
			_vTarget = _effectTarget;
			break;
		}
		case State::Rotate:
		{
			// そのまま一定速度回転（不要なら角度も mymath でイージング可能）
			float sx = _vPos.x - _vTarget.x;
			float sz = _vPos.z - _vTarget.z;
			float length = sqrtf(sx * sx + sz * sz);
			if (length > 1e-6f)
			{
				float rad = atan2f(sz, sx);
				rad += _rotateSpeed * dt;
				_vPos.x = _vTarget.x + cosf(rad) * length;
				_vPos.z = _vTarget.z + sinf(rad) * length;
			}
			break;
		}
		case State::Shake:
		{
			float progress = (_duration > 0.0f) ? (_timer / _duration) : 1.0f;
			float amp = _shakeIntensity * (1.0f - progress);
			float rx = ((float)rand() / RAND_MAX * 2.0f - 1.0f) * amp;
			float ry = ((float)rand() / RAND_MAX * 2.0f - 1.0f) * amp * 0.5f;
			float rz = ((float)rand() / RAND_MAX * 2.0f - 1.0f) * amp;
			_vPos.x += rx;
			_vPos.y += ry;
			_vPos.z += rz;
			break;
		}
		default:
			break;
	}
	// 演出終了判定
	if (_timer >= _duration)
	{
		_state = State::Idle;
		_timer = 0.0f;
		_duration = 0.0f;
	}
	return true;
}

bool CinematicCamera::Render()
{
	// デバッグ情報（必要なら有効化）
	// 基本は Camera::Render と同等の情報を描画しても良い
	return true;
}

void CinematicCamera::StartOrbit(const vec::Vec3& target, float durationSeconds, float startRadius, float endRadius, float revolutions)
{
	_state = State::Orbit;
	_timer = 0.0f;
	_duration = durationSeconds;
	_effectTarget = target;
	_orbitStartRadius = startRadius;
	_orbitEndRadius = endRadius;
	_orbitRevolutions = revolutions;
	
	// 現在のカメラ位置から見たターゲットの角度を計算して保存
	float sx = _vPos.x - _effectTarget.x;
	float sz = _vPos.z - _effectTarget.z;
	_orbitStartAngle = atan2f(sz, sx);

}

void CinematicCamera::StartZoom(const vec::Vec3& target, float durationSeconds, float startDist, float endDist)
{
	_state = State::Zoom;
	_timer = 0.0f;
	_duration = durationSeconds;
	_effectTarget = target;
	_zoomStartDist = startDist;
	_zoomEndDist = endDist;
}

void CinematicCamera::StartRotateSpeed(float radiansPerSec, float durationSeconds)
{
	_state = State::Rotate;
	_timer = 0.0f;
	_duration = durationSeconds;
	_rotateSpeed = radiansPerSec;
}

void CinematicCamera::StartShake(float intensity, float durationSeconds)
{
	_state = State::Shake;
	_timer = 0.0f;
	_duration = durationSeconds;
	_shakeIntensity = intensity;
}

void CinematicCamera::StopAll()
{
	_state = State::Idle;
	_timer = 0.0f;
	_duration = 0.0f;
}
