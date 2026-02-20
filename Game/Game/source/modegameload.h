#pragma once
#include "appframe.h"

class ModeGameLoad : public ModeBase
{
	typedef ModeBase base;
public:
	ModeGameLoad();
	~ModeGameLoad();

	bool Initialize();
	bool Terminate();
	bool Process();
	bool Render();

protected:
	int _loadHandle;
};

