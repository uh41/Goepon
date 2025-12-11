#pragma once
#include "charabase.h"
class EnemyBase : public CharaBase
{
	typedef CharaBase base;

public:

	virtual bool Initialize();
	virtual bool Terminate();
	virtual bool Process();
	virtual bool Render();



protected:

};

