#pragma once
#include "enemybase.h"
#include "appframe.h"
#include "movepointcontroll.h"

// ‘O•ûéŒ¾
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

	void SetEnemySensor(std::shared_ptr<EnemySensor> sensor);
};

