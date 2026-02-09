#include "enemysoundsensor.h"
#include "appframe.h"
#include "map.h"

bool EnemySoundSensor::Initialize()
{
	base::Initialize();

	_soundsensorarea.center = _vPos;	// センサーの中心を初期化

	return true;
}

bool EnemySoundSensor::Terminate()
{
	base::Terminate();
	return true;
}

bool EnemySoundSensor::Process()
{
	base::Process();
	
	_soundsensorarea.center = _vPos;	// センサーの中心を敵の位置に更新

	// 音波の更新
	UpdateSoundWaves();

	// 検出タイマーの更新
	if (_sounddetectionInfo.timer > 0.0f)
	{
		_sounddetectionInfo.timer -= 1.0f / 60.0f;

		// タイマーが0以下になったら検出状態をリセット
		if (_sounddetectionInfo.timer <= 0.0f)
		{
			_sounddetectionInfo.isDetected = false;
		}
	}

	return true;
}

void EnemySoundSensor::SetSoundSensorArea(float radius)
{
	_soundsensorarea.radius = radius;
}

void EnemySoundSensor::SetSoundLevel(int level)
{
	_soundLevel = level;
}

// 音波の発生
void EnemySoundSensor::TriggerSoundWave(const vec::Vec3& origin, float maxRadius, float speed)
{
	
	SoundWave* wave = nullptr;

	// 非アクティブな音波を探す
	for (auto& w : _soundWaves)
	{
		if (!w.isActive)
		{
			wave = &w;
			break;
		}
	}

	// 非アクティブな音波が見つからなければ、新しい音波を追加
	if (!wave && _soundWaves.size() < MAX_SOUND_WAVES) 
	{
		_soundWaves.emplace_back();
		wave = &_soundWaves.back();
	}

	// 音波の初期化
	if (wave) 
	{
		wave->origin = origin;
		wave->currentRadius = 0.0f;
		wave->maxRadius = maxRadius;
		wave->expandSpeed = speed;
		wave->isActive = true;
		wave->alpha = 1.0f;
		wave->soundLevel = _soundLevel;
	}
}

// 音波の更新
void EnemySoundSensor::UpdateSoundWaves()
{
	for (auto& wave : _soundWaves)
	{
		if (!wave.isActive) continue;

		// 音波の拡大
		wave.currentRadius += wave.expandSpeed;

		// 透明度の更新
		wave.alpha = 1.0f - (wave.currentRadius / wave.maxRadius);

		// 衝突判定
		if (CheckSoundWaveCollision(wave))
		{
			// 音検知処理
			OnSoundDetected(wave.origin);
		}

		// 音波の終了判定
		if (wave.currentRadius >= wave.maxRadius)
		{
			wave.isActive = false;
		}
	}
}

// 音波とセンサー範囲の衝突判定
bool EnemySoundSensor::CheckSoundWaveCollision(const SoundWave& wave) const
{
	// センサー範囲が無効な場合は衝突しない
	if (_soundsensorarea.radius <= 0.0f) 
	{
		return false;
	}

	// 音波の中心とセンサーの中心の距離を計算
	vec::Vec3 waveCenter = wave.origin;
	vec::Vec3 sensorCenter = _soundsensorarea.center;

	// 水平距離のみを考慮
	float dx = waveCenter.x - sensorCenter.x;
	float dz = waveCenter.z - sensorCenter.z;
	float centerDistance = sqrtf(dx * dx + dz * dz);

	// 音波の中心がセンサー範囲内にある場合は即座に衝突とみなす
	if (centerDistance <= _soundsensorarea.radius)
	{
		return true;
	}

	// 衝突判定
	float radiusSum = wave.currentRadius + _soundsensorarea.radius;

	// 音波がセンサー範囲に入ったかどうか
	if (centerDistance <= radiusSum)
	{
		// 前フレームで衝突していなかったか確認
		float prevRadius = wave.currentRadius - wave.expandSpeed;
		float prevRadiusSum = prevRadius + _soundsensorarea.radius;

		// 前フレームで衝突していなかった場合、衝突とみなす
		if (prevRadiusSum < centerDistance) 
		{
			return true;
		}
	}

	return false;
}

// 音検知時の処理
void EnemySoundSensor::OnSoundDetected(const vec::Vec3& soundOrigin)
{
	// 既に検出されている場合は処理しない
	if (_sounddetectionInfo.isDetected)
	{
		return;
	}

	// 音の大きさレベルに応じて検出情報を更新
	if (_soundLevel == 5)
	{
		_sounddetectionInfo.isDetected = true;
		_sounddetectionInfo.timer = 2.0f; 
		_sounddetectionInfo.soundSourcePosition = soundOrigin;  
		_sounddetectionInfo.detectedSoundLevel = _soundLevel;  
	}
}

// 音波の描画
void EnemySoundSensor::RenderSoundWaves()
{
	for (const auto& wave : _soundWaves)
	{
		if (!wave.isActive) continue;

		// 円周を分割するセグメント数
		constexpr int segments = 32;
		constexpr float angleStep = (2.0f * DX_PI_F) / segments;

		// 色と透明度の設定
		unsigned int color = GetColor(255, 255, 0);
		int alpha = static_cast<int>(wave.alpha * 255);
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);

		// 音波の高さを少し上げる
		float y = wave.origin.y + 0.5f;  

		// 円周上の点を計算して線分を描画
		for (int i = 0; i < segments; ++i)
		{
			float angle1 = angleStep * i;
			float angle2 = angleStep * (i + 1);

			VECTOR pos1 = VGet(
				wave.origin.x + wave.currentRadius * cosf(angle1),
				y,
				wave.origin.z + wave.currentRadius * sinf(angle1)
			);
			VECTOR pos2 = VGet(
				wave.origin.x + wave.currentRadius * cosf(angle2),
				y,
				wave.origin.z + wave.currentRadius * sinf(angle2)
			);

			DrawLine3D(pos1, pos2, color);
		}

		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	}
}

// デバッグ用：音センサー範囲の描画
bool EnemySoundSensor::Render()
{
	base::Render();
	
	if (_soundsensorarea.radius > 0.0f)
	{
		// センサー範囲の色設定
		unsigned int color;

		// 音が検出されている場合は赤、そうでなければ黄色
		if (_sounddetectionInfo.isDetected)
		{
			color = GetColor(255, 0, 0); 
		}
		else
		{
			color = GetColor(255, 255, 0);
		}

		const int segments = 32;
		const float angleStep = (2.0f * DX_PI_F) / segments;

		// センサー範囲の中心と半径
		vec::Vec3 center = _soundsensorarea.center;
		float radius = _soundsensorarea.radius;

		// 円周を描画
		for (int i = 0; i < segments; i++)
		{
			float angle1 = angleStep * i;
			float angle2 = angleStep * (i + 1);

			
			vec::Vec3 pos1 = vec3::VAdd(center, vec3::VGet(
				cosf(angle1) * radius,
				0.0f,
				sinf(angle1) * radius
			));
			vec::Vec3 pos2 = vec3::VAdd(center, vec3::VGet(
				cosf(angle2) * radius,
				0.0f,
				sinf(angle2) * radius
			));

			DxlibConverter::DrawLine3D(pos1, pos2, color);

			// 上方向にも線分を描画して立体感を出す
			vec::Vec3 pos1_up = vec3::VAdd(pos1, vec3::VGet(0.0f, 10.0f, 0.0f));
			vec::Vec3 pos2_up = vec3::VAdd(pos2, vec3::VGet(0.0f, 10.0f, 0.0f));
			DxlibConverter::DrawLine3D(pos1_up, pos2_up, color);
		}

		// 十字マーカーの描画
		vec::Vec3 marker1 = vec3::VAdd(center, vec3::VGet(-15.0f, 5.0f, 0.0f));
		vec::Vec3 marker2 = vec3::VAdd(center, vec3::VGet(15.0f, 5.0f, 0.0f));
		vec::Vec3 marker3 = vec3::VAdd(center, vec3::VGet(0.0f, 5.0f, -15.0f));
		vec::Vec3 marker4 = vec3::VAdd(center, vec3::VGet(0.0f, 5.0f, 15.0f));

		DxlibConverter::DrawLine3D(marker1, marker2, color);
		DxlibConverter::DrawLine3D(marker3, marker4, color);
	}

	// 音波の描画
	RenderSoundWaves();

	return true;
}

