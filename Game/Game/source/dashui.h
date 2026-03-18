#pragma once
#include "uibase.h"

class PlayerTanuki;

namespace dash
{
	static constexpr auto DASH_ICON_X = 1690; // ダッシュアイコンのX座標
	static constexpr auto DASH_ICON_Y = 890; // ダッシュアイコンのY座標
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

	void SetPlayer(PlayerTanuki* player) { _player = player; } // プレイヤーへのポインタをセットするセッター

protected:
	PlayerTanuki* _player; // プレイヤーへのポインタ
};

