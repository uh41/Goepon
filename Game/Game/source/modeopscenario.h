#pragma once
#include "modescenariobase.h"

class ModeOpScenario : public ModeScenarioBase
{
	public:
	ModeOpScenario();
	virtual ~ModeOpScenario();
	virtual bool Initialize()override;
	virtual bool Terminate()override;
	virtual bool Process()override;
	virtual bool Render()override;
};

