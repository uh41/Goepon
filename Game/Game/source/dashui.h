#pragma once
#include "uibase.h"

class PlayerBase;

namespace dash
{
	static constexpr auto DASH_ICON_X = 1696; // ダッシュアイコンのX座標
	static constexpr auto DASH_ICON_Y = 902; // ダッシュアイコンのY座標
}

class DashUi : public UiBase
{
	typedef UiBase base;
public:	
	DashUi();
	virtual ~DashUi() = default;
	bool Initialize() override;
	bool Terminate() override;
	bool Process() override;
	bool Render() override;

	void SetPlayer(PlayerBase* player) { _player = player; } // プレイヤーへのポインタをセットするセッター

protected:
	int _noDashHandle;
	PlayerBase* _player; // プレイヤーへのポインタ
};

