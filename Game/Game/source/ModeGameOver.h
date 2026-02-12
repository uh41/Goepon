#pragma once
#include "appframe.h"

class ModeGameOver :public ModeBase
{
	typedef ModeBase base;
public:
	explicit ModeGameOver(ModeBase* ownerGame = nullptr) : _ownerGame(ownerGame) {} // コンストラクタ

	virtual bool Initialize() override;
	virtual bool Terminate() override;
	virtual bool Process() override;
	virtual bool Render() override;

private:
	ModeBase* _ownerGame; // 所有しているゲームモードへのポインタ
};

