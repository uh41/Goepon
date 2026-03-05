#include "treasureopenui.h"
#include "modegame.h"

TreasureOpenUi::TreasureOpenUi()
{
	_handle = -1;
	_pos = vec::Vec3(0.0f, 0.0f, 0.0f);
	_visible = false;
	_size = 100.0f;
	_angle = 0.0f;
}

bool TreasureOpenUi::Initialize()
{
	base::Initialize();
	_handle = LoadGraph(ui::KeyOpen);
	return true;
}

bool TreasureOpenUi::Terminate()
{
	base::Terminate();
	if(_handle != -1)
	{
		DeleteGraph(_handle);
		_handle = -1;
	}
	return true;
}

bool TreasureOpenUi::Process()
{
	base::Process();
	return true;
}

bool TreasureOpenUi::Render()
{
	base::Render();

	if(!_visible || _handle == -1)
	{
		return false;
	}

	auto modeGame = dynamic_cast<ModeGame*>(ModeServer::GetInstance()->Get("game"));
	if(!modeGame)
	{
		return false;
	}

	// 現在表示中のプレイヤーに応じて座標を取得
	vec::Vec3 playerPos;
	bool hasPlayer = false;

	if(modeGame->IsShowingTanuki())
	{
		// タヌキ状態
		auto playerTanuki = modeGame->GetPlayerTanuki();
		if(playerTanuki)
		{
			playerPos = playerTanuki->GetPos();
			hasPlayer = true;
		}
	}
	else
	{
		// 人間状態（またはモノ状態）
		auto player = modeGame->GetPlayer();
		if(player)
		{
			playerPos = player->GetPos();
			hasPlayer = true;
		}
	}

	if(hasPlayer)
	{
		// Y座標を上にオフセット（プレイヤーの頭上に表示）
		playerPos.x += treasureopen::OPEN_UI_X;

		VECTOR pos = DxlibConverter::VecToDxLib(playerPos);
		DrawBillboard3D(pos, 0.5f, 0.5f, _size, _angle, _handle, TRUE);
	}

	return true;
}