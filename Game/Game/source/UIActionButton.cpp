#include "UIActionButton.h"

UIActionButton::UIActionButton(ModeGame* owner)
	: _ownerGame(owner)
	, _playerBase(nullptr)
	, _handOffsetY(0.0f)
	, _drawW(64)
	, _drawH(64)
{
	
}

bool UIActionButton::Initialize()
{
	base::Initialize();
	_handOffsetY = 100.0f; //  プレイヤーの上あたりに出す想定
	return true;
}

bool UIActionButton::Terminate()
{
	base::Terminate();

	return true;
}

bool UIActionButton::Process()
{
	base::Process();
	if (!_playerBase) { return true; }
	return true;
}

bool UIActionButton::Render()
{
	base::Render();
	if(!_playerBase) { return true; }
	if(handle == -1) { return true; }

	// TODO: 3D->2D変換があるならここで変換する
	// int screenX, screenY; = ...
	
	// 位置
	const vec::Vec3 playerPos = _playerBase->GetPos();

	// TODO: 実際の描画APIに合わせて変更（例：DrawGraph / DrawExtendGraph）
	// DrawExtendGraph(x, y, x + _drawW, y + _drawH, handle, TRUE);

	return true;
}