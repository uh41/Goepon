#include "playermono.h"

PlayerMono::PlayerMono()
{
	Initialize();
}

PlayerMono::~PlayerMono()
{

}

bool PlayerMono::Initialize()
{
	if(!base::Initialize()) { return false; }
	_handle = MV1LoadModel(mv1::tuzura_02);
	_iAttachIndex = -1;

	_status = STATUS::NONE;

	_fTotalTime = 0.0f;
	_fPlayTime = 0.0f;

	_fColSubY = 40.0f;
}

bool PlayerMono::Terminate()
{
	base::Terminate();
	return true;
}

bool PlayerMono::Process()
{
	base::Process();
	return true;
}

bool PlayerMono::Render()
{
	base::Render();
	return true;
}

