#pragma once
#include "appframe.h"

class ModeGameOverLoad : public ModeBase
{
	typedef ModeBase base;
public:
	ModeGameOverLoad();
	virtual ~ModeGameOverLoad();
	virtual bool Initialize() override;
	virtual bool Terminate() override;
	virtual bool Process() override;
	virtual bool Render() override;

protected:
	ModeBase* _owner;
	int _handle;
	int _frameShow;
};

