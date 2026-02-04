#include "enemysoundsensor.h"
#include "appframe.h"
#include "map.h"

bool EnemySoundSensor::Initialize()
{
	base::Initialize();

	_soundsensorarea.center = _vPos;

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
	// センサーの中心を敵の現在位置に更新
	_soundsensorarea.center = _vPos;

	UpdateSoundWaves();

	// 検出表示タイマーの更新
	if (_sounddetectionInfo.timer > 0.0f)
	{
		_sounddetectionInfo.timer -= 1.0f / 60.0f; // 60FPSとして計算

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

// 音の波紋を発生させる
void EnemySoundSensor::TriggerSoundWave(const vec::Vec3& origin, float maxRadius, float speed)
{
	// 既存の非アクティブな波紋を再利用、またはベクターに追加
	SoundWave* wave = nullptr;

	// 既存の非アクティブな波紋を探す
	for (auto& w : _soundWaves)
	{
		if (!w.isActive) 
		{
			wave = &w;
			break;
		}
	}

	// 新しい波紋を追加
	if (!wave && _soundWaves.size() < MAX_SOUND_WAVES)
	{
		_soundWaves.emplace_back();
		wave = &_soundWaves.back();
	}

	// 波紋の初期化
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

// 音の波紋の更新
void EnemySoundSensor::UpdateSoundWaves()
{
	for (auto& wave : _soundWaves) 
	{
		if (!wave.isActive) continue;

		// 半径を拡大
		wave.currentRadius += wave.expandSpeed;

		// 透明度を減少（最大半径に近づくほど薄くなる）
		wave.alpha = 1.0f - (wave.currentRadius / wave.maxRadius);

		// ★ 音の波紋とサウンドセンサーの当たり判定
		if (CheckSoundWaveCollision(wave))
		{
			// 音を検知した場合の処理
			OnSoundDetected(wave.origin);
		}

		// 最大半径に達したら非アクティブに
		if (wave.currentRadius >= wave.maxRadius)
		{
			wave.isActive = false;
		}
	}
}

// 音の波紋とサウンドセンサーの円の当たり判定
bool EnemySoundSensor::CheckSoundWaveCollision(const SoundWave& wave) const
{
	// サウンドセンサーが無効または半径が未設定の場合は判定しない
	if (_soundsensorarea.radius <= 0.0f) 
	{
		return false;
	}

	// 2つの円の中心間の距離を計算（Y軸は無視して2D平面で計算）
	vec::Vec3 waveCenter = wave.origin;
	vec::Vec3 sensorCenter = _soundsensorarea.center;

	// XZ平面での距離計算
	float dx = waveCenter.x - sensorCenter.x;
	float dz = waveCenter.z - sensorCenter.z;
	float centerDistance = sqrtf(dx * dx + dz * dz);

	// 2つの円が接触または重なっているかチェック
	// 波紋の半径 + センサーの半径 >= 中心間の距離
	float radiusSum = wave.currentRadius + _soundsensorarea.radius;

	// 波紋がセンサー範囲に到達または重なっている場合
	if (centerDistance <= radiusSum) 
	{
		// さらに、波紋がセンサー範囲の外側から来ている場合のみ検知
		// （センサー内部で発生した音は除外する場合）
		float prevRadius = wave.currentRadius - wave.expandSpeed;
		float prevRadiusSum = prevRadius + _soundsensorarea.radius;

		// 前フレームでは接触していなかったが、今フレームで接触した場合
		if (prevRadiusSum < centerDistance) 
		{
			return true;
		}
	}

	return false;
}

// 音を検知したときの処理
void EnemySoundSensor::OnSoundDetected(const vec::Vec3& soundOrigin)
{
	// 既に検知状態の場合は更新しない（連続して検知しないようにする）
	if (_sounddetectionInfo.isDetected) 
	{
		return;
	}

	// 音レベルが5の場合のみ反応
	if (_soundLevel == 5)
	{
		// 検知状態を設定
		_sounddetectionInfo.isDetected = true;
		_sounddetectionInfo.timer = 2.0f; // 例: 2秒間検知状態を維持
		_sounddetectionInfo.soundSourcePosition = soundOrigin;  // 音源の位置を保存
		_sounddetectionInfo.detectedSoundLevel = _soundLevel;  // 音レベルを保存
	}
}

void EnemySoundSensor::RenderSoundWaves()
{
	for (const auto& wave : _soundWaves) 
	{
		if (!wave.isActive) continue;

		// 円の分割数
		constexpr int segments = 32;
		constexpr float angleStep = (2.0f * DX_PI_F) / segments;

		// 色設定（例: 黄色の波紋）
		unsigned int color = GetColor(255, 255, 0);
		int alpha = static_cast<int>(wave.alpha * 255);
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);

		// 円を線分で描画（3D空間でのY座標は地面+少し上）
		float y = wave.origin.y + 0.5f;  // 地面より少し浮かせる

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

bool EnemySoundSensor::Render()
{
	base::Render();
	// デバッグ用にセンサー範囲を描画（円）
	if (_soundsensorarea.radius > 0.0f)
	{
		unsigned int color;

		// 円の色（黄色）
		if (_sounddetectionInfo.isDetected)
		{
			color = GetColor(255, 0, 0); // 検出中は赤
		}	
		else 
		{
			color = GetColor(255, 255, 0);
		}
		

		// 円を描画するための分割数
		const int segments = 32;
		const float angleStep = (2.0f * DX_PI_F) / segments;

		// センサー中心位置
		vec::Vec3 center = _soundsensorarea.center;
		float radius = _soundsensorarea.radius;

		// 円周上の点を描画
		for (int i = 0; i < segments; i++)
		{
			float angle1 = angleStep * i;
			float angle2 = angleStep * (i + 1);

			// 円周上の2点を計算
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

			// 線を描画
			DxlibConverter::DrawLine3D(pos1, pos2, color);

			// 少し上の位置にも線を描画して見えやすくする
			vec::Vec3 pos1_up = vec3::VAdd(pos1, vec3::VGet(0.0f, 10.0f, 0.0f));
			vec::Vec3 pos2_up = vec3::VAdd(pos2, vec3::VGet(0.0f, 10.0f, 0.0f));
			DxlibConverter::DrawLine3D(pos1_up, pos2_up, color);
		}

		// 中心点を示すマーカーを描画
		vec::Vec3 marker1 = vec3::VAdd(center, vec3::VGet(-15.0f, 5.0f, 0.0f));
		vec::Vec3 marker2 = vec3::VAdd(center, vec3::VGet(15.0f, 5.0f, 0.0f));
		vec::Vec3 marker3 = vec3::VAdd(center, vec3::VGet(0.0f, 5.0f, -15.0f));
		vec::Vec3 marker4 = vec3::VAdd(center, vec3::VGet(0.0f, 5.0f, 15.0f));

		// 十字マーカーの描画
		DxlibConverter::DrawLine3D(marker1, marker2, color);
		DxlibConverter::DrawLine3D(marker3, marker4, color);
	}

	RenderSoundWaves();

	return true;
}

