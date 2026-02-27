#pragma once
#include "appframe.h"

class ModeGameOverLoad : public ModeBase
{
	typedef ModeBase base;
public:
	explicit ModeGameOverLoad(ModeBase* ownerGame, const std::string& stageId);
	virtual ~ModeGameOverLoad();
	virtual bool Initialize() override;
	virtual bool Terminate() override;
	virtual bool Process() override;
	virtual bool Render() override;

protected:
	ModeBase* _owner;
	std::string _stageId; // 再開するステージID
	int _handle;
	int _frameShow;
	bool _requestedReset;
	bool _spawnedGame; // ModeGameを一度だけ生成するためのフラグ
};