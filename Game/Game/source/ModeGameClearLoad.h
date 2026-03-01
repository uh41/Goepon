#pragma once
#include "appframe.h"
#include "modegame.h"
#include "applicationglobal.h"
#include "StageManager.h"

class ModeGameClearLoad : public ModeBase
{
	typedef ModeBase base;
public:
	explicit ModeGameClearLoad(ModeBase* ownerGame, const std::string& stageId);
	virtual ~ModeGameClearLoad();

	virtual bool Initialize() override;
	virtual bool Terminate() override;
	virtual bool Process() override;
	virtual bool Render() override;

protected:
	ModeBase* _owner;
	std::string _stageId;
	int _handle;
	int _frameShow;
	bool _requestedReset;
	bool _spawnedGame;
	std::unique_ptr<StageManager> _stageManager;
};

