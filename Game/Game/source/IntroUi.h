#pragma once
#include "uibase.h"

class ModeGame;

namespace ui
{
	static constexpr auto INTROLOGO_X = 200; // ゲームスタートUIのX座標
	static constexpr auto INTROLOGO_Y = 700; // ゲームスタートUIのY座標
}

class IntroUi :public UiBase
{
	typedef UiBase base;
public:
	IntroUi();
	virtual ~IntroUi() = default;
	virtual bool Initialize() override;
	virtual bool Terminate() override;
	virtual bool Process() override;
	virtual bool Render() override;
	
	void SetOwner(ModeGame* owner) { _ownerGame = owner; }	

protected:
	ModeGame* _ownerGame;	
};

