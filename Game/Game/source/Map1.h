#pragma once
#include "MapBase.h"

class Map1 : public MapBase
{
	typedef	MapBase base;
public:
	virtual bool Initialize();
	virtual bool Terminate();
	virtual bool Process();
	virtual bool Render();
};

