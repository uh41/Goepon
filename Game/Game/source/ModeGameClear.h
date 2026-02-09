#pragma once
#include "modegame.h"
#include "appframe.h"
class ModeGameClear :public ModeBase
{
	typedef ModeBase base;
public:
	virtual bool Initialize() override;
	virtual bool Terminate() override;
	virtual bool Process() override;
	virtual bool Render() override;

private:
	int _fontSize;
};

