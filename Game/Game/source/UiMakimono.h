#pragma once
#include "uibase.h"
#include "Player.h"

class UiMakimono :public UiBase
{
	typedef UiBase base;
public:
	UiMakimono();
	virtual ~UiMakimono();

bool Initialize()override;
bool Terminate()override;
bool Process()override;
bool Render()override;

private:
	// 右下配置用
	int _padding = 15;     // 画面端からの余白
	float _scale = 1.0f;   // 必要なら 0.5f とかにして縮小
};

