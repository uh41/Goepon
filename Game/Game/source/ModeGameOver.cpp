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
		// セーブが存在すれば読み込んで既存の ModeGame に適用（既存の "game" は削除しない）
		SaveData sd{};
		if(SaveManager::TryLoad(sd, SaveManager::GetDefaultPath()))
		{
			ModeBase* existing = ModeServer::GetInstance()->Get("game");
			if(existing)
			{
				auto* game = dynamic_cast<ModeGame*>(existing);
				if(game)
				{
					game->ApplySaveData(sd);
					game->ResetEnemiesToInitialPositions(); // 敵を初期位置に戻す（セーブデータの位置に合わせるため）
					// GameOver モードだけ閉じる（既存の game をそのまま残す）

					//game->ResetEnemyRoot(); // 敵のルートをリセットしてセーブデータの位置に合わせる
					ModeServer::GetInstance()->Del(this);
					return true;
				}
			}

			// 既存の game が無ければ新規生成してセーブのステージで起動
			auto* newGame = new ModeGame();
			newGame->SetInitialStageId(sd.stageId);
			ModeServer::GetInstance()->Add(newGame, 0, "game");
			ModeServer::GetInstance()->ProcessInit();

			// Initialize 後に取得してセーブ内容を適用（ApplySaveData はタヌキ開始に固定する）
			{
				ModeBase* gm = ModeServer::GetInstance()->Get("game");
				if(gm)
				{
					auto* game = dynamic_cast<ModeGame*>(gm);
					if(game)
					{
						game->ApplySaveData(sd);
						game->ResetEnemiesToInitialPositions();
						//game->ResetEnemyRoot();
					}
				}
			}

			ModeServer::GetInstance()->Del(this);
			return true;
		}

		// セーブが無ければ従来の挙動へフォールバック（既存の game を削除しない）
		if(ModeServer::GetInstance()->Get("gameoverload") == nullptr)
		{
			ModeServer::GetInstance()->Add(new ModeGameOverLoad(nullptr, _debugCurrentStageId), 300, "gameoverload");
			ModeServer::GetInstance()->ProcessInit();
		}
		ModeServer::GetInstance()->Del(this);
		return true;
	}

	return true;
}

bool ModeGameOver::Render()
{
	base::Render();

	// デバッグ情報の表示（Render内で行う）
	//DrawFormatString(10, 50, GetColor(255, 255, 0), "DEBUG: Detected Stage ID = %s", _debugCurrentStageId.c_str());
	//DrawFormatString(10, 70, GetColor(255, 255, 0), "DEBUG: _ownerGame = %s", _ownerGame ? "Valid" : "NULL");

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