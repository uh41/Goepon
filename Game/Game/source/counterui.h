#pragma once
#include "uibase.h"

namespace counter
{
	static constexpr auto COUNTER_TREASURE_X = 1500;//1809.0f
	static constexpr auto COUNTER_TREASURE_Y = 100;//100.0f
	static constexpr auto COUNTER_MAKIMONO_X = 1809;
	static constexpr auto COUNTER_MAKIMONO_Y = 145;
	static constexpr auto COUNTER_TREASURE_NEXT_X = 25;
	static constexpr auto COUNTER_MAKIMONO_NEXT_X = 25;
}

class CounterUi : public UiBase
{
	typedef UiBase base;
public:
	CounterUi();
	virtual ~CounterUi() = default;
	virtual bool Initialize() override;
	virtual bool Terminate() override;
	virtual bool Process() override;
	virtual bool Render() override;

	void RenderNumber(int number, int x, int y,bool drawZero = true);

	int GetTreasureCount() const { return _treasureCount; }
	void SetTreasureCount(int count) { _treasureCount = count; }

	void DecreaseTreasureCount();

protected:
	at::art<int, 10> _handleNum; // ”š‰æ‘œ
	int _treasureCount; // •ó•¨‚ÌƒJƒEƒ“ƒg
};

