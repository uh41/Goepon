#pragma once
#include "MapBase.h"

class Map2 : public MapBase
{
	typedef	MapBase base;
public:
	virtual bool Initialize();
	virtual bool Terminate();
	virtual bool Process();
	virtual bool Render();
};

