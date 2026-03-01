#include "ModeGameClearLoad.h"

ModeGameClearLoad::ModeGameClearLoad(ModeBase* owneeGame, const std::string& stageId)
{
	_owner = owneeGame;
	_stageId = stageId.empty() ? "Stage1" : stageId; // デフォルトのステージID
	_handle = -1;
	_frameShow = 0;
	_requestedReset = false;
	_spawnedGame = false;
}

ModeGameClearLoad::~ModeGameClearLoad()
{

}

bool ModeGameClearLoad::Initialize()
{
	base::Initialize();
	_handle = LoadGraph(img::gameoverload);
	return true;
}

bool ModeGameClearLoad::Terminate()
{
	base::Terminate();
	if(_handle != -1)
	{
		DeleteGraph(_handle);
		_handle = -1;
	}
	return true;
}

bool ModeGameClearLoad::Process()
{
	base::Process();
	 //ModeGameが存在するかを確認
	ModeBase* gameBase = ModeServer::GetInstance()->Get("game");
	if (gameBase == nullptr)
	{
		// gameがないなら待機してから新しく追加してロード追加
		if(!_spawnedGame)
		{
			// 保存されたステージIDを使用して新しいModeGameを作成
			auto* newGame = new ModeGame();
			newGame->SetInitialStageId(_stageId);

			ModeServer::GetInstance()->Add(newGame, 0, "game");
			ModeServer::GetInstance()->ProcessInit();
			_spawnedGame = true;
		}
		return true;
	}

	// gameが存在する場合、ModeGameにキャストしてロード完了を確認
	auto* game = dynamic_cast<ModeGame*>(gameBase);
	if (game == nullptr) return false;

	// 新しく追加された ModeGame のロードが完了したらオーバーレイを削除する
	if (game->IsLoadComplete())
	{
		ModeServer::GetInstance()->Del(this);
	}
	return true;
}

bool ModeGameClearLoad::Render()
{
	base::Render();

	_frameShow++;

	// デバッグ: Render 呼び出し確認表示（画面左上）
	DrawFormatString(10, 10, GetColor(0, 255, 255), "ModeGameClearLoad Render frame=%d handle=%d", _frameShow, _handle);

	if (_handle != -1)
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
		DrawString(10, 50, "GameClear Loading...", GetColor(0, 255, 255));
	}
	return true;
}