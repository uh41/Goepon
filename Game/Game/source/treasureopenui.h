#pragma once
#include "uibase.h"

class ModeGame;

namespace treasureopen
{
	static constexpr auto OPEN_UI_X = 45.0f; // 宝箱オープンUIのX座標
}

class TreasureOpenUi : public UiBase
{
	typedef UiBase base;
public:
	TreasureOpenUi();
	virtual ~TreasureOpenUi() = default;
	virtual bool Initialize() override;
	virtual bool Terminate() override;
	virtual bool Process() override;
	virtual bool Render() override;

};

