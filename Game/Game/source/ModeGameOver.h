#pragma once
#include "appframe.h"
#include "modegame.h"

class ModeGameOver :public ModeBase
{
	typedef ModeBase base;
public:
	// オーナーを受け取るコンストラクタを用意（nullptr可）
	explicit ModeGameOver(ModeGame* ownerGame = nullptr)
		: _ownerGame(ownerGame)
		, _overlayHandle(-1)
		, _showOverlayImmediate(false)
	{
	}

	virtual bool Initialize() override;
	virtual bool Terminate() override;
	virtual bool Process() override;
	virtual bool Render() override;

private:
	ModeGame* _ownerGame; // 所有している ModeGame への参照（必要に応じて使用）
	int _overlayHandle;
	bool _showOverlayImmediate;
};