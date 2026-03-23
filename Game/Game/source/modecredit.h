#pragma once
#include "appframe.h"

namespace credit
{
	static constexpr auto CREDIT_X = 56; // スキップ表示開始X座標
	static constexpr auto CREDIT_Y = 1012; // スキップ表示開始Y座標
}

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

	int _creditHandle; // クレジットの画像ハンドル

};

