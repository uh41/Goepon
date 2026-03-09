#include "ModeGameClear.h"
#include "ApplicationMain.h"
#include "modegame.h"
#include "modeteamlogo.h"
#include "ModeGameClearLoad.h" 

bool ModeGameClear::Initialize()
{
	if(!base::Initialize()) return false;

	if(gGlobal._soundServer)
	{
		gGlobal._soundServer->StopType(soundserver::SoundItemBase::TYPE::BGM);
	}
	return true;
}

bool ModeGameClear::Terminate()
{
	base::Terminate();
	return true;
}

bool ModeGameClear::Process()
{
	base::Process();
	// クリア画面が出ている間は「そのほかのレイヤー」を止める
	ModeServer::GetInstance()->SkipProcessUnderLayer();
	// キー取得
	int trg = ApplicationBase::GetInstance()->GetTrg();

	if(trg & PAD_INPUT_1)
	{
		// 現在のステージIDを保存
		std::string currentStageId = "Stage1"; // デフォルトのステージID
		if (_ownerGame)
		{
			auto* game = dynamic_cast<ModeGame*>(_ownerGame);
			if(game)
			{
				currentStageId = game->GetCurrentStageId();
			}
		}

		//if(_ownerGame)
		//{
		//	ModeServer::GetInstance()->Del(_ownerGame); // ゲームモードを削除
		//}

		// 名前"game" で登録されているモードがあれば削除予約
		ModeBase* existing = ModeServer::GetInstance()->Get("game");
		if (existing)
		{
			ModeServer::GetInstance()->Del(existing);
		}

		// 自分自身を削除予約（先に削除して、ModeGameClearLoadが上に来るようにする）
		ModeServer::GetInstance()->Del(this);

		// ModeGameClearLoadを使用してゲームを再ロード
		// layer を 100 に下げて、確実に上に描画されるようにする
		if(ModeServer::GetInstance()->Get("gameclearload") == nullptr)
		{
			ModeServer::GetInstance()->Add(new ModeGameClearLoad(nullptr, currentStageId), 100, "gameclearload");
		}

		// 削除・追加は次フレームの ModeServer::ProcessInit() で実行されるため、
		// ここでは早期リターンして安全に終了する。
		return true;
	}
	return true;
}

bool ModeGameClear::Render()
{
	base::Render();

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

	// クリアメッセージ
	SetFontSize(TitleFontSize);
	DrawString
	(
		TitlePosX, TitlePosY,
		ClearMessage,
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

	return true;
}
