#include "modegameload.h"
#include "modegame.h"

ModeGameLoad::ModeGameLoad()
{
	_loadHandle = -1;
	_effectHandle = -1;
	_playHandle = -1;
}

ModeGameLoad::~ModeGameLoad()
{
}

bool ModeGameLoad::Initialize()
{
	if(!base::Initialize()) { return false; }
	_loadHandle = ResourceServer::LoadGraph(img::Loading_1start);
	Fade::GetInstance()->ColorMask(0, 0, 0, 0);

	auto em = EffekseerManager::GetInstance();
	if(em)
	{
		_effectHandle = em->LoadEffect(ef::EF_doya);
		int screenW, screenH;
		GetScreenState(&screenW, &screenH, nullptr);
		vec::Vec3 centerPos(screenW / 2.0f, screenH / 2.0f, 0.0f);
		_playHandle = em->PlayEffect2DPos(_effectHandle, centerPos);
	}
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

	auto em = EffekseerManager::GetInstance();
	if(em)
	{
		if(_playHandle != -1)
		{
			em->StopEffect2D(_playHandle);
			_playHandle = -1;
		}
		if(_effectHandle != -1)
		{
			em->DeleteEffect(_effectHandle);
			_effectHandle = -1;
		}
	}

	return true;
}

bool ModeGameLoad::Process()
{
	base::Process();

	// エフェクトがロード済みで、再生が停止している場合は再度再生
	auto em = EffekseerManager::GetInstance();
	if(em && _effectHandle != -1)
	{
		if(_playHandle == -1 || !em->IsPlayingEffect2D(_playHandle))
		{
			int screenW, screenH;
			GetScreenState(&screenW, &screenH, nullptr);
			vec::Vec3 centerPos(screenW / 2.0f, screenH / 2.0f, 0.0f);
			_playHandle = em->PlayEffect2DPos(_effectHandle, centerPos);
		}
	}

	// まだゲームが居なければ追加（1回だけ）
	ModeBase* gameBase = ModeServer::GetInstance()->Get("game");
	if(gameBase == nullptr)
	{
		if(GetASyncLoadNum() == 0)
		{
			ModeServer::GetInstance()->Add(new ModeGame(), 1, "game");
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