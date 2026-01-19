#pragma once
#include "appframe.h"
class modetitle : public ModeBase
{
	public:
	virtual bool Initialize();
	virtual bool Terminate();
	virtual bool Process();
	virtual bool Render();
};

