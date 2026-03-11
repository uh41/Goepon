#pragma once
#include "appframe.h"
#include "TitleTanuki.h"
#include "TitleCamera.h"
#include "applicationglobal.h"

namespace soundserver
{
	class SoundServer;
}

namespace ui
{
	static constexpr auto START_CONFIG_X = 0;
	static constexpr auto START_CONFIG_Y = 980;
}

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
		
	at::spc<soundserver::SoundServer> _soundServer;
	int _gbmHandle;
	int _startHandle;
};

