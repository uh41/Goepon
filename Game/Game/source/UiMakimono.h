#pragma once
#include "PlayerBase.h"
#include "counterui.h"

namespace makimono
{
	static constexpr auto MAKIMONO_X = 1646; // 巻物のX座標 1646
	static constexpr auto MAKIMONO_Y = 876; // 巻物のY座標 146
	static constexpr auto KAKERU_X = 1746; // 掛ける画像X座標 1746
	static constexpr auto KAKERU_Y = 876; // 掛ける画像Y座標 146
	static constexpr auto MAKIMONO_COUNT_X = 1800; // 巻物のカウントX座標 1809
	static constexpr auto MAKIMONO_COUNT_Y = 870; // 巻物のカウントY座標 145
}

class UiMakimono :public CounterUi
{
	typedef CounterUi base;
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

	int _handleMakimono;
	int _handleUiX;
};

