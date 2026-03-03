#pragma once
#include "uibase.h"
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

	void RenderCount(int open, int total);

protected:
	at::art<int, 10> _handleNum; // ”š‰æ‘œ
};

