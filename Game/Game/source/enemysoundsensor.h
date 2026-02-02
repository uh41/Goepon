#pragma once
#include "enemybase.h"
#include "playerbase.h"

// 前方宣言
class Map;

struct SoundSensorArea
{
	vec::Vec3 center;   // センサーの中心点（敵の位置）
	float radius;       // センサー範囲の半径
};

class EnemySoundSensor : public EnemyBase
{
	typedef EnemyBase base;

public:
	virtual bool Initialize();
	virtual bool Terminate();
	virtual bool Process();
	virtual bool Render();

	void SetMap(Map* map) { _map = map; }

	// サウンドセンサー範囲の設定
	void SetSoundSensorArea(float radius);	// 半径
protected:
	SoundSensorArea _soundsensorarea;
	Map* _map;
};

