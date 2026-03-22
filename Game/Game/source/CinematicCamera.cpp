#include "CinematicCamera.h"
#include "appframe.h"

CinematicCamera::CinematicCamera()
{
	_state = State::Idle;
	_timer = 0.0f;
	_duration = 0.0f;
	_targetPos = vec::Vec3{ 0.0f, 0.0f, 0.0f };
	_orbitStartRadius = _orbitEndRadius = _orbitRevolutions = _orbitStartAngle = 0.0f; 
	_zoomStartDist = _zoomEndDist = 0.0f;
	_zoomEasing = &mymath::EasingOutBounce;
	_rotateSpeed = 0.0f;
	_shakeIntensity = 0.0f;
	_shakeCyclesPerSecond = 8.0f;
	_shakeBasePos = vec::Vec3{ 0.0f, 0.0f, 0.0f };

}

bool CinematicCamera::Initialize()
{
	base::Initialize(); // Camera の初期化も行う
	_state = State::Idle;
	_timer = 0.0f;
	_duration = 0.0f;
	return true;
}

bool CinematicCamera::Terminate()
{
	base::Terminate(); // Camera の終了処理も行う
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
		// 円軌道を描いて回る
		case State::Orbit:
		{
			// 半径を mymath のイージングで補間
			float radius = mymath::EasingOutQuad(cnt, _orbitStartRadius, _orbitEndRadius, frames);

			// 回転進行量もイージングしたい場合は revolutions をイージング（0->_orbitRevolutions）
			float easedRevs = mymath::EasingInOutQuad(cnt, 0.0f, _orbitRevolutions, frames);
			float angle = _orbitStartAngle + 2.0f * DX_PI_F * easedRevs;

			_vTarget = _targetPos;
			_vPos.x = _targetPos.x + cosf(angle) * radius;
			_vPos.z = _targetPos.z + sinf(angle) * radius;
			_vPos.y = _targetPos.y + 200.0f;
			break;
		}
		// ターゲットに向かって距離を変える
		case State::Zoom:
		{
			const auto easing = (_zoomEasing != nullptr) ? _zoomEasing : &mymath::EasingOutBounce;
			float dist = easing(cnt, _zoomStartDist, _zoomEndDist, frames);

			// Back等で負になるのを防ぐ
			if(dist < 0.0f)
			{
				dist = 0.0f;
			}
			vec::Vec3 dir = vec3::VSub(_vPos, _targetPos);
			float len = vec3::VSize(dir);
			if(len > 1e-6f)
			{
				vec::Vec3 nd = vec3::VScale(dir, 1.0f / len); // 正規化	
				_vPos = vec3::VAdd(_targetPos, vec::Vec3{ nd.x * dist, nd.y * dist, nd.z * dist });
			}
			_vTarget = _targetPos;
			break;
		}
		// 回転速度一定
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
		// 揺れ（ランダムオフセットを加える）
		case State::Shake:
		{
			const float progress = (_duration > 0.0f) ? (_timer / _duration) : 1.0f; 
			const float amp      = _shakeIntensity * (1.0f - progress); // 徐々に減衰させる

			const float t = _timer; //seconds
			const float yOffSet = sinf(2.0f * DX_PI_F * _shakeCyclesPerSecond * t) * amp;

			_vPos = _shakeBasePos;
			_vPos.y += yOffSet; // Y軸方向に揺らす。必要ならX/Zも同様に揺らす
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
	return true;
}

void CinematicCamera::StartOrbit(const vec::Vec3& target, float durationSeconds, float startRadius, float endRadius, float revolutions)
{
	_state = State::Orbit;
	_timer = 0.0f;
	_duration = durationSeconds;
	_targetPos = target;
	_orbitStartRadius = startRadius;
	_orbitEndRadius = endRadius;
	_orbitRevolutions = revolutions;
	
	// 現在のカメラ位置から見たターゲットの角度を計算して保存
	float sx = _vPos.x - _targetPos.x;
	float sz = _vPos.z - _targetPos.z;
	_orbitStartAngle = atan2f(sz, sx);

}

void CinematicCamera::StartZoom(const vec::Vec3& target, float durationSeconds, float startDist, float endDist, EasingFunc easing)
{
	_state = State::Zoom;
	_timer = 0.0f;
	_duration = durationSeconds;
	_targetPos = target;
	_zoomStartDist = startDist;
	_zoomEndDist = endDist;
	_zoomEasing = (easing != nullptr) ? easing : &mymath::EasingOutBounce; // デフォルトはバウンス
}

void CinematicCamera::StartRotateSpeed(float radiansPerSec, float durationSeconds)
{
	_state       = State::Rotate;
	_timer       = 0.0f;
	_duration	 = durationSeconds;
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
