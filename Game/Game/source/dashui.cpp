#include "dashui.h"
#include "playertanuki.h"
#include "playerfactory.h"

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

	PlayerBase* tanuki = PlayerFactory::GetTanukiPlayer();
	if(!tanuki)
	{
		return false;
	}

	auto* tanukiPlayer = dynamic_cast<PlayerTanuki*>(tanuki);
	if(!tanukiPlayer)
	{
		return false;
	}

	// クールタイム中に表示する
	if(tanukiPlayer != nullptr && _noDashHandle != -1)
	{
		float cd = tanukiPlayer->GetDashCoolDownTime();

		if(cd > 0.0f || _player == nullptr)
		{
			DrawGraph(dash::DASH_ICON_X, dash::DASH_ICON_Y, _noDashHandle, TRUE);
		}
	}
	return true;
}