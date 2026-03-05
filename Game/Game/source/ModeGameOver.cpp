#include "ModeGameOver.h"
#include "ApplicationMain.h"
#include "modegame.h"
#include "modegameoverload.h"

bool ModeGameOver::Initialize()
{
	if(!base::Initialize()) return false;

	_debugCurrentStageId = "Stage1"; // デフォルト
	if(_ownerGame)
	{
		auto* game = dynamic_cast<ModeGame*>(_ownerGame);
		if(game)
		{
			_debugCurrentStageId = game->GetCurrentStageId();
		}
	}

	return true;
}

bool ModeGameOver::Terminate()
{
	// 即時描画用にロードしたグラフを解放
	if(_overlayHandle != -1)
	{
		DeleteGraph(_overlayHandle);
		_overlayHandle = -1;
		_showOverlayImmediate = false;
	}

	base::Terminate();
	return true;
}

bool ModeGameOver::Process()
{
	base::Process();
	ModeServer::GetInstance()->SkipProcessUnderLayer();

	//ModeServer::GetInstance()->SkipProcessUnderLayer();
	int trg = ApplicationMain::GetInstance()->GetTrg();

	if(trg & PAD_INPUT_1)
	{
		// 1) 現在のステージIDを保存（削除前に取得）
		std::string currentStageId = "Stage1"; // デフォルト値
		if(_ownerGame)
		{
			auto* game = dynamic_cast<ModeGame*>(_ownerGame);
			if(game)
			{
				currentStageId = game->GetCurrentStageId();
				// デバッグ出力：取得したステージID
				DrawFormatString(10, 100, GetColor(255, 0, 0), "DEBUG: Current Stage ID = %s", currentStageId.c_str());
			}
			else
			{
				DrawFormatString(10, 100, GetColor(255, 0, 0), "DEBUG: Failed to cast _ownerGame to ModeGame");
			}
		}
		else
		{
			DrawFormatString(10, 100, GetColor(255, 0, 0), "DEBUG: _ownerGame is null, using default Stage1");
		}
		// 2) 所有している ModeGame があれば削除予約（安全に予約する）
		if(_ownerGame)
		{
			ModeServer::GetInstance()->Del(_ownerGame);
			_ownerGame = nullptr; // 所有参照を切る
		}

		// 3) 名前 "game" で登録されているモードがあれば削除予約
		ModeBase* existing = ModeServer::GetInstance()->Get("game");
		if(existing)
		{
			ModeServer::GetInstance()->Del(existing);
		}

		// デバッグ出力：最終的に使用するステージID
		DrawFormatString(10, 140, GetColor(255, 255, 0), "DEBUG: Final Stage ID = %s", currentStageId.c_str());

		// 4) オーバーレイを予約追加時に現在のステージIDを渡す
		if(ModeServer::GetInstance()->Get("gameoverload") == nullptr)
		{
			ModeServer::GetInstance()->Add(new ModeGameOverLoad(nullptr, currentStageId), 300, "gameoverload");
			ModeServer::GetInstance()->ProcessInit();
		}

		// 5) 自分自身を削除予約
		ModeServer::GetInstance()->Del(this);

		// 削除・追加は次フレームの ModeServer::ProcessInit() で実行されるため、
		// ここでは早期リターンして安全に終了する。
		return true;
	}

	return true;
}

bool ModeGameOver::Render()
{
	base::Render();

	// デバッグ情報の表示（Render内で行う）
	DrawFormatString(10, 50, GetColor(255, 255, 0), "DEBUG: Detected Stage ID = %s", _debugCurrentStageId.c_str());
	DrawFormatString(10, 70, GetColor(255, 255, 0), "DEBUG: _ownerGame = %s", _ownerGame ? "Valid" : "NULL");

	// 背景(半透明に設定)
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, BackgroundAlpha);
	DrawBox
	(
		BgLeft, BgTop, BgRight, BgBottom,
		GetColor(BlackR, BlackG, BlackB),
		TRUE
	);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

	// 枠
	DrawBox
	(
		BgLeft, BgTop, BgRight, BgBottom,
		GetColor(255, 255, 255),
		FALSE
	);

	// ゲームオーバーメッセージ
	SetFontSize(TitleFontSize);
	DrawString
	(
		TitlePosX, TitlePosY,
		GameOverMessage,
		GetColor(WhiteR, WhiteG, WhiteB)
	);

	// ヒントメッセージ
	SetFontSize(HintFontSize);
	DrawString
	(
		HintPosX, HintPosY,
		HintMessage,
		GetColor(HintR, HintG, HintB)
	);

	// PAD 押下直後の即時表示（ModeGameOverLoad がまだ初期化されていないフレーム向け）
	if(_showOverlayImmediate && _overlayHandle != -1)
	{
		int screenW, screenH;
		GetScreenState(&screenW, &screenH, nullptr);

		int imgW, imgH;
		GetGraphSize(_overlayHandle, &imgW, &imgH);

		int x = (screenW - imgW) / 2;
		int y = (screenH - imgH) / 2;

		// overlay を前面に描画
		DrawGraph(x, y, _overlayHandle, TRUE);
	}

	return true;
}