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
	return true;
}

void EnemySoundSensor::SetSoundSensorArea(float radius)
{
	_soundsensorarea.radius = radius;
}

bool EnemySoundSensor::Render()
{
	base::Render();
	// デバッグ用にセンサー範囲を描画（円）
	if (_soundsensorarea.radius > 0.0f)
	{
		// 円の色（黄色）
		unsigned int color = GetColor(255, 255, 0);

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
	return true;
}

