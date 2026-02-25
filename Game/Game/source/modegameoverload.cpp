#include "modegameoverload.h"
#include "ModeGame.h"
#include "applicationglobal.h"

ModeGameOverLoad::ModeGameOverLoad()
{
	_owner = nullptr;
	_handle = -1;
	_frameShow = 0;
}

ModeGameOverLoad::~ModeGameOverLoad()
{

}

bool ModeGameOverLoad::Initialize()
{
	if (!base::Initialize()) return false;

	_handle = LoadGraph(img::gameoverload);

	return true;
}

bool ModeGameOverLoad::Terminate()
{
	base::Terminate();
	if (_handle != -1)
	{
		DeleteGraph(_handle);
		_handle = -1;
	}
	return true;
}

bool ModeGameOverLoad::Process()
{
	base::Process();
	ModeBase* gameBase = ModeServer::GetInstance()->Get("game");
	if(gameBase == nullptr)
	{
		// game が無ければ待つ（もしくは別途 ModeGame を生成する設計にもできるが今回は既存 game を想定）
		return true;
	}

	auto* game = dynamic_cast<ModeGame*>(gameBase);
	if(game == nullptr) return true;
	if(game->HasRenderOnce() && _frameShow >= 1)
	{
		ModeServer::GetInstance()->Del(this);
	}

	return true;
}

bool ModeGameOverLoad::Render()
{
	base::Render();

	_frameShow++;

	if(_handle != -1)
	{
		// 画像を画面中央に描画
		int screenW, screenH;
		GetScreenState(&screenW, &screenH, nullptr);

		int imgW, imgH;
		GetGraphSize(_handle, &imgW, &imgH);

		int x = (screenW - imgW) / 2;
		int y = (screenH - imgH) / 2;

		DrawGraph(x, y, _handle, TRUE);
	}
	else
	{
		// フォールバック表示（デバッグ用）
		DrawString(10, 10, "GameOver overlay missing", GetColor(255, 0, 0));
	}
	return true;
}