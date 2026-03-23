/*********************************************************************/
// * \file   enemy.h
// * \brief  エネミークラス
// *
// * \author 市村義春
/*********************************************************************/

#pragma once
#include "enemybase.h"
#include "appframe.h"
#include "movepointcontroll.h"

// 前方宣言
class EnemySensor;
class PlayerBase;

class Enemy : public EnemyBase
{
	typedef EnemyBase base;

public:
	virtual bool Initialize();
	virtual bool Terminate();
	virtual bool Process();
	virtual bool Render();

	// 音検知の半径をオーバーライド
	virtual float GetHearingRadius() const override { return 300.0f; } 
};
