#pragma once
#include "enemybase.h"
#include "appframe.h"
#include "movepointcontroll.h"

// 前方宣言
class EnemySensor;
class EnemySoundSensor;
class PlayerBase;

class EnemyDog : public EnemyBase
{
	typedef EnemyBase base;
public:
public:
	virtual bool Initialize();
	virtual bool Terminate();
	virtual bool Process();
	virtual bool Render();

	// 初期位置と向きをキャプチャ
	void CaptureInitialTransform();

	void SetEnemySensor(std::shared_ptr<EnemySensor> sensor);
};

