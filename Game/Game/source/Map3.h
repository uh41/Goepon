#pragma once
#include "MapBase.h"

class Map3 : public MapBase
{
	typedef MapBase base;
public:
	virtual bool Initialize() override;
	virtual bool Terminate() override;
	virtual bool Process() override;
	virtual bool Render() override;
};

