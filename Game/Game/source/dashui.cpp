#include "dashui.h"
#include "playertanuki.h"

DashUi::DashUi()
{
	_noDashHandle = -1;
	_player = nullptr;
}

bool DashUi::Initialize()
{
	base::Initialize();
	_noDashHandle = LoadGraph(ui::Dash_Icon);
	return true;
}

bool DashUi::Terminate()
{
	base::Terminate();
	if(_noDashHandle != -1)
	{
		DeleteGraph(_noDashHandle);
		_noDashHandle = -1;
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

	// クールタイム中に表示する
	if(_player != nullptr && _noDashHandle != -1)
	{
		float cd = _player->GetDashCoolDownTime();

		if(cd > 0.0f)
		{
			DrawGraph(dash::DASH_ICON_X, dash::DASH_ICON_Y, _noDashHandle, TRUE);
		}
	}
	return true;
}