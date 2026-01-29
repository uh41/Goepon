#pragma once
#include "appframe.h"

class ModeScenarioBase : public ModeBase
{
public:
	struct Page
	{
		int handle = -1;
		std::string voiceFile;
	};

	ModeScenarioBase();
	virtual ~ModeScenarioBase();
	virtual bool Initialize()override;
	virtual bool Terminate()override;
	virtual bool Process()override;
	virtual bool Render()override;

	void ChangePage(int pageNo);
protected:
	at::vec<Page> _page;
	int _pageNo;
	at::spc<soundserver::SoundServer> _soundServer;
	soundserver::SoundItemVoice* _voice;
};

