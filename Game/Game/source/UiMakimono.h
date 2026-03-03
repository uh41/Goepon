#pragma once
#include "uibase.h"
#include "PlayerBase.h"

namespace makimono
{
	static constexpr auto MAKIMONO_X = 1554.0f; // 巻物のX座標
	static constexpr auto MAKIMONO_Y = 20.0f; // 巻物のY座標

}

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

void SetPlayer(PlayerBase* player) { _player = player; }
private:

	// プレイヤーの情報を取得
	PlayerBase* _player;

	// 右下配置用
	int _padding = 15;     // 画面端からの余白
	float _scale = 1.0f;   // 必要なら 0.5f とかにして縮小
};

