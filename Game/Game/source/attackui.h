#pragma once
#include "uibase.h"

class ModeGame;

namespace attack
{
	static constexpr auto PLAYER_HEAD_Y = 200.0f; // プレイヤーの頭の高さ（UIの位置調整用）
}

class AttackUi : public UiBase
{
	typedef UiBase base;
public:
	AttackUi();
	virtual ~AttackUi() = default;
	virtual bool Initialize() override;
	virtual bool Terminate() override;
	virtual bool Process() override;
	virtual bool Render() override;

protected:


};

