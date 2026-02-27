#pragma once
#include "uibase.h"

namespace henshin
{
	static constexpr auto MAKIMONO_X = 1670;
	static constexpr auto MAKIMONO_Y = 500;
}

class HenshinUi : public UiBase
{
	typedef UiBase base;
public:
	HenshinUi();
	virtual ~HenshinUi() = default;
	virtual bool Initialize();
	virtual bool Terminate();
	virtual bool Process();
	virtual bool Render();
};

