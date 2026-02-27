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

	// owner を使った RequestResetStage() は使わない仕様に変更したため削除しました。

	// ModeGame が存在するか確認
	ModeBase* gameBase = ModeServer::GetInstance()->Get("game");
	if(gameBase == nullptr)
	{
		// game が無ければ一度だけ新しく追加してロードを開始する
		if(!_spawnedGame)
		{
			ModeServer::GetInstance()->Add(new ModeGame(), 0, "game");
			// 同フレームで初期化を行い描画リストへ移す（オーバーレイ表示と同時に初期化したい場合）
			ModeServer::GetInstance()->ProcessInit();
			_spawnedGame = true;
		}
		// 追加直後はロードが進むまで待つ
		return true;
	}

	auto* game = dynamic_cast<ModeGame*>(gameBase);
	if(game == nullptr) return true;

	// 新しく追加された ModeGame がロード完了したらオーバーレイを削除する
	if(game->IsLoadComplete())
	{
		ModeServer::GetInstance()->Del(this);
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