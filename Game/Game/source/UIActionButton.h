#pragma once
#include "uibase.h"
#include "modegame.h"
#include "playerBase.h"

class UIActionButton : public UiBase
{
	typedef UiBase base;
public:
	explicit UIActionButton(ModeGame* owner);

	virtual bool Initialize() override;
	virtual bool Terminate() override;
	virtual bool Process() override;
	virtual bool Render() override;

	void SetPlayer(PlayerBase* player) { _playerBase = player; }
private:
	ModeGame* _ownerGame;
	PlayerBase* _playerBase;

	// Œ©‚½–Ú/ˆÊ’u
	float _handOffsetY;
	int _drawW = 64;
	int _drawH = 64;
};

