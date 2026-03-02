#pragma once
#include "appframe.h"
#include "TitleTanuki.h"
#include "TitleCamera.h"
class ModeTitle : public ModeBase
{
	typedef ModeBase base;
public:
	ModeTitle();
	virtual ~ModeTitle();
	virtual bool Initialize() override;
	virtual bool Terminate() override;
	virtual bool Process() override;
	virtual bool Render() override;

protected:
	at::spc<TitleTanuki> _player;
	TitleCamera* _cam;
};

