#include "henshinui.h"
#include "appframe.h"
#include "applicationglobal.h"

HenshinUi::HenshinUi()
{
	_handle = -1;
	Initialize();
}

bool HenshinUi::Initialize()
{
	base::Initialize();
	_handle = ResourceServer::LoadGraph(img::UI_Makimono);
	return true;
}

bool HenshinUi::Terminate()
{
	base::Terminate();

	if(_handle)
	{
		DeleteGraph(_handle);
		_handle = -1;
	}

	return true;
}

bool HenshinUi::Process()
{
	base::Process();
	return true;
}

bool HenshinUi::Render()
{
	base::Render();

	if(_handle != -1)
	{
		DrawGraph(henshin::MAKIMONO_X, henshin::MAKIMONO_Y, _handle, TRUE);
	}

	return true;
}
