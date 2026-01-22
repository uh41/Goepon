#pragma once
#include "appframe.h"

class ModeInit : public ModeBase
{
public:

	ModeInit();
	virtual ~ModeInit();
	virtual bool Initialize();
	virtual bool Terminate();
	virtual bool Process();
	virtual bool Render();
};

