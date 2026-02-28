#include "modegameload.h"
#include "modegame.h"

ModeGameLoad::ModeGameLoad()
{
	_loadHandle = -1;
}

ModeGameLoad::~ModeGameLoad()
{
}

bool ModeGameLoad::Initialize()
{
	if(!base::Initialize()) { return false; }
	_loadHandle = ResourceServer::LoadGraph(img::Loading_1start);
	Fade::GetInstance()->ColorMask(0, 0, 0, 0);
	return true;
}

bool ModeGameLoad::Terminate()
{
	base::Terminate();
	if(_loadHandle != -1)
	{
		DeleteGraph(_loadHandle);
		_loadHandle = -1;
	}
	return true;
}

bool ModeGameLoad::Process()
{
	base::Process();

	// まだゲームが居なければ追加（1回だけ）
	ModeBase* gameBase = ModeServer::GetInstance()->Get("game");
	if(gameBase == nullptr)
	{
		if(GetASyncLoadNum() == 0)
		{
			ModeServer::GetInstance()->Add(new ModeGame(), 0, "game");
		}
		return true;
	}

	// ゲームが 1 回でも Render 済みならロードを消す
	auto* game = dynamic_cast<ModeGame*>(gameBase);
	if(game != nullptr && game->HasRenderOnce())
	{
		ModeServer::GetInstance()->Del(this);
	}

	return true;
}

bool ModeGameLoad::Render()
{
	if(_loadHandle != -1)
	{
		int screenW, screenH;
		GetScreenState(&screenW, &screenH, nullptr);

		int imgW, imgH;
		GetGraphSize(_loadHandle, &imgW, &imgH);

		int x = (screenW - imgW) / 2;
		int y = (screenH - imgH) / 2;

		DrawGraph(x, y, _loadHandle, TRUE);
	}
	else
	{
		DrawString(10, 10, "Loading...", GetColor(255, 255, 255));
	}

	return true;
}