#pragma once
#include "appframe.h"

class ModeCredit : public ModeBase
{
	public:
	ModeCredit();
	virtual ~ModeCredit() = default;
	virtual bool Initialize() override;
	virtual bool Terminate() override;
	virtual bool Process() override;
	virtual bool Render() override;

protected:
	bool _isStart;// クレジットが開始されたかどうか

};

