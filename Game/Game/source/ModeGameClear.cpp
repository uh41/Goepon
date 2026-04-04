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
		// 全BGMを停止
		gGlobal._soundServer->StopType(soundserver::SoundItemBase::TYPE::BGM);

		_handle = LoadGraph(ui::Gameclear); 

		if(auto s = gGlobal._soundServer->Get("bgminitialize"))
		{
			s->Stop();
		}
		if(auto s2 = gGlobal._soundServer->Get("bgmChenge"))
		{
			s2->Stop();
		}

		// クリアBGMが鳴っていなければ再生
		if(auto clearBgm = gGlobal._soundServer->Get("160"))
		{
			if(!clearBgm->IsPlay())
			{
				clearBgm->Play();
			}
		}
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
	// クリア画面が出ている間はほかのレイヤーを止める
	ModeServer::GetInstance()->SkipProcessUnderLayer();

	// キー取得
	int trg = ApplicationBase::GetInstance()->GetTrg();

	if(trg & PAD_INPUT_1)
	{
		// 現在のステージIDを保存
		std::string currentStageId = "Stage1"; // デフォルトのステージID

		// オーナーゲームから現在のステージIDを取得して保存しておく
		if (_ownerGame)
		{
			auto* game = dynamic_cast<ModeGame*>(_ownerGame);
			if(game)
			{
				currentStageId = game->GetCurrentStageId();
			}
		}

		// 次ステージがある場合のみ、クリアBGMを止める
		if(gGlobal._soundServer)
		{
			StageManager sm;
			sm.SetStages(gGlobal.GetStageList());
			const bool hasNextStage = !sm.GetNextStageId(currentStageId).empty();

			if(hasNextStage)
			{
				if(auto clearBgm = gGlobal._soundServer->Get("160"))
				{
					clearBgm->Stop();
				}
			}
		}

		//　"game" で登録されているモードがあれば削除予約
		ModeBase* existing = ModeServer::GetInstance()->Get("game");
		if (existing)
		{
			ModeServer::GetInstance()->Del(existing);
		}

		// 自分自身を削除予約
		ModeServer::GetInstance()->Del(this);

		// 次のステージがある場合は、ゲームクリアロード でロードモードを登録
		if(ModeServer::GetInstance()->Get("gameclearload") == nullptr)
		{
			ModeServer::GetInstance()->Add(new ModeGameClearLoad(nullptr, currentStageId), 100, "gameclearload");
		}

		return true;
	}
	return true;
}

bool ModeGameClear::Render()
{
	base::Render();

	if(!_handle)
	{
		return false;
	}

	int x = 250, y = 100;

	DrawGraph(x, y, _handle, TRUE);

	return true;
}
