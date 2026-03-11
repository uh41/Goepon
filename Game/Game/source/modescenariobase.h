#pragma once
#include "appframe.h"

namespace ui
{

	static constexpr auto MOVE_X = 0;
	static constexpr auto MOVE_Y = 1030;
}

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
	int _backHandle; // 背景のグラフィックハンドル	
	at::spc<soundserver::SoundServer> _soundServer;
	at::spc<soundserver::SoundItemVoice> _voice;

	int _moveHandle;
};

