#include "treasureopenui.h"
#include "modegame.h"
#include "playerform.h"

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

	// 現在表示中のプレイヤーを取得（PlayerForm から）
	PlayerBase* currentPlayer = PlayerForm::GetInstance()->GetPlayer();

	if(currentPlayer)
	{
		// Y座標を上にオフセット（プレイヤーの頭上に表示）
		vec::Vec3 playerPos = currentPlayer->GetPos();
		playerPos.x += treasureopen::OPEN_UI_X;

		VECTOR pos = DxlibConverter::VecToDxLib(playerPos);
		DrawBillboard3D(pos, 0.5f, 0.5f, _size, _angle, _handle, TRUE);
	}

	return true;
}