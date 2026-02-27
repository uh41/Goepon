#include "modegameoverload.h"
#include "ModeGame.h"
#include "applicationglobal.h"

ModeGameOverLoad::ModeGameOverLoad(ModeBase* ownerGame)
{
	_owner = ownerGame;
	_handle = -1;
	_frameShow = 0;
	_requestedReset = false;
	_spawnedGame = false;
}

ModeGameOverLoad::~ModeGameOverLoad()
{

}

bool ModeGameOverLoad::Initialize()
{
	if(!base::Initialize()) return false;

	_handle = LoadGraph(img::gameoverload);

	return true;
}

bool ModeGameOverLoad::Terminate()
{
	base::Terminate();
	if(_handle != -1)
	{
		DeleteGraph(_handle);
		_handle = -1;
	}
	return true;
}

bool ModeGameOverLoad::Process()
{
	base::Process();

	// ModeGameが保存するか確認
	ModeBase* gameBase = ModeServer::GetInstance()->Get("game");
	if(gameBase == nullptr)
	{
		// gameがなければ一度だけ新しく追加してロードを開始
		if(!_spawnedGame)
		{
			std::string currentStageId = "Stage1"; //　デフォルト

			// オーナーがModeGameの場合、現在のステージ情報を取得
			if(_owner != nullptr)
			{
				auto* ownerGame = dynamic_cast<ModeGame*>(_owner);
				if(ownerGame != nullptr)
				{
					currentStageId = ownerGame->GetCurrentStageId(); // オーナーから現在のステージIDを取得
				}
			}

			// 現在のステージIDを指定して新しいModeGameを作成
			auto* newGame = new ModeGame();
			newGame->SetInitialStageId(currentStageId);

			ModeServer::GetInstance()->Add(newGame, 0, "game"); 
			ModeServer::GetInstance()->ProcessInit(); // 追加したモードのInitializeを呼び出す
			_spawnedGame = true;
		}
		// 追加直後はロードが進むまで待つ
		return true;
	}
	auto* game = dynamic_cast<ModeGame*>(gameBase);
	if(game == nullptr) return false;

	// 新しく追加された ModeGame がロード完了したらオーバーレイを削除する
	if(game->IsLoadComplete())
	{
		ModeServer::GetInstance()->Del(this); // 自分自身を削除予約	
	}
	return true;
}

bool ModeGameOverLoad::Render()
{
	base::Render();

	_frameShow++;

	// デバッグ: Render 呼び出し確認表示（画面左上）
	DrawFormatString(10, 10, GetColor(255, 255, 0), "ModeGameOverLoad Render frame=%d handle=%d", _frameShow, _handle);

	if(_handle != -1)
	{
		// 画像を画面中央に描画
		int screenW, screenH;
		GetScreenState(&screenW, &screenH, nullptr);

		int imgW, imgH;
		GetGraphSize(_handle, &imgW, &imgH);

		// デバッグ: 画像サイズ表示
		DrawFormatString(10, 30, GetColor(255, 255, 0), "imgW=%d imgH=%d", imgW, imgH);

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