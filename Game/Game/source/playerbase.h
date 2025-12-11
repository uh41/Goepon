#pragma once
#include "charabase.h"
class PlayerBase : public CharaBase
{
	typedef CharaBase base;
public:
	virtual bool Initialize();
	virtual bool Terminate();
	virtual bool Process();
	virtual bool Render();

protected:

};

