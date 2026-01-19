#pragma once
#include "appframe.h"
class ModeTitle : public ModeBase
{
public:
	ModeTitle();
	virtual ~ModeTitle();
	virtual bool Initialize() override;
	virtual bool Terminate() override;
	virtual bool Process() override;
	virtual bool Render() override;
};

