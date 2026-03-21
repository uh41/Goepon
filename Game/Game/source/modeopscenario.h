#pragma once
#include "modescenariobase.h"

class ModeOpScenario : public ModeScenarioBase
{
	typedef ModeScenarioBase base;
public:
	ModeOpScenario();
	virtual ~ModeOpScenario();
	virtual bool Initialize()override;
	virtual bool Terminate()override;
	virtual bool Process()override;
	virtual bool Render()override;

private:
	// 漫画風コマ表示
	struct ComicPanel
	{
		int x, y;		// 表示位置
		//int w, h;		// 表示サイズ
		float alpha;	// 透明度（0.0f - 1.0f）
	};

	at::vec<ComicPanel> _panelData;    // 漫画風コマのリスト
	int _fadeTimer;					   // フェード用タイマー
	static const int FADE_FRAME = 30;  // フェード時間

	
};

