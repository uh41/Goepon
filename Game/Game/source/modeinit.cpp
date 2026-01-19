#include "modeinit.h"

ModeInit::ModeInit()
{
	Initialize();

	Fade::GetInstance()->ColorMask(0, 0, 0, 255);		// カラーマスクの設定
}

ModeInit::~ModeInit()
{
	Terminate();
}

bool ModeInit::Initialize()
{
	_iHandle = LoadGraph("res/logo/AMGlogo.png");
	return true;
}

bool ModeInit::Terminate()
{
	if(_iHandle != -1)
	{
		DeleteGraph(_iHandle);
		_iHandle = -1;
	}
	return true;
}

bool ModeInit::Process()
{
	return true;
}

bool ModeInit::Render()
{
	DrawGraph(0, 0, _iHandle, TRUE);
	return true;
}

