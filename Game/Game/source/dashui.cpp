#include "dashui.h"
#include "playertanuki.h"

DashUi::DashUi()
{
	_handle = -1;
	_player = nullptr;
}

bool DashUi::Initialize()
{
	base::Initialize();
	_handle = LoadGraph(ui::Dash_Icon);
	return true;
}

bool DashUi::Terminate()
{
	base::Terminate();
	if(_handle != -1)
	{
		DeleteGraph(_handle);
		_handle = -1;
	}
	return true;
}

bool DashUi::Process()
{
	base::Process();
	return true;
}

bool DashUi::Render()
{
	base::Render();

	//if(!_visible)
	//{
	//	return false;
	//}

	if(_player != nullptr && _handle != -1)
	{
		bool canDash = (!_player->IsDash()) &&
			(_player->GetDashCoolDownTime() <= 0.0f) &&
			(_player->GetDashCount() < dash::DASH_MAX);

		if(canDash)
		{
			DrawGraph(dash::DASH_ICON_X, dash::DASH_ICON_Y, _handle, TRUE);
		}
	}

	
	return true;
}