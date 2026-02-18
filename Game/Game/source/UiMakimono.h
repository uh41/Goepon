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
	void SetPlayer(PlayerBase* player) { _player = player; }

private:
	PlayerBase* _player; // プレイヤーへのポインタ（まきものの所持数を取得するため）


};

