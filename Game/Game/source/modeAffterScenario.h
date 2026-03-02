#pragma once
#include "ModeScenarioBase.h"

class ModeAfScenario : public ModeScenarioBase
{
public:
	ModeAfScenario();
	virtual ~ModeAfScenario();
	virtual bool Initialize()override;
	virtual bool Terminate()override;
	virtual bool Process()override;
	virtual bool Render()override;
};

