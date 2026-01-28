#pragma once
#include "appframe.h"

class ModeTeamLogo : public ModeBase
{
public:
	ModeTeamLogo();
	virtual ~ModeTeamLogo();
	virtual bool Initialize()override;
	virtual bool Terminate()override;
	virtual bool Process()override;
	virtual bool Render()override;

};

