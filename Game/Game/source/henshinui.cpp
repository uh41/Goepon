#include "henshinui.h"
#include "appframe.h"
#include "applicationglobal.h"
#include "modegame.h"
#include "playermanager.h"
#include "playerform.h"

HenshinUi::HenshinUi()
{
	_makimono        = nullptr;
	_select			 = Select::TANUBITO;
	_padInput5Active = true;
	_owner			 = nullptr;

	_showPlayerUi	  = false;
	_showPlayerMonoUi = false;

	_handleMain = -1;
	_handleNotCg    = -1;
	_handleTanubito = -1;
	_handleMono     = -1;

}

bool HenshinUi::Initialize()
{
	base::Initialize();

	_handleMain     = LoadGraph(ui::UI_Hito           );
	_handleNotCg	= LoadGraph(ui::UI_Hito_Makimono_0);
	_handleTanubito = LoadGraph(ui::UI_Tanubito_no    );
	_handleMono	    = LoadGraph(ui::UI_Mono           );
	return true;
}

bool HenshinUi::Terminate()
{
	base::Terminate();

	if(_handleMain != -1)
	{
		DeleteGraph(_handleMain);
		_handleMain = -1;
	}
	if(_handle != -1)
	{
		DeleteGraph(_handleNotCg);
		_handleNotCg = -1;
	}
	if(_handleTanubito != -1)
	{
		DeleteGraph(_handleTanubito);
		_handleTanubito = -1;
	}
	if(_handleMono != -1)
	{
		DeleteGraph(_handleMono);
		_handleMono = -1;
	}

	return true;
}

bool HenshinUi::Process()
{
	base::Process();

	int key = ApplicationBase::GetInstance()->GetKey();
	int trg = ApplicationBase::GetInstance()->GetTrg();

	// ModeGameを参照して変身中かどうかを確認
	ModeGame* mg = nullptr;
	if(_owner)
	{
		mg = StCas<ModeGame*>(_owner);
	}

	auto* playerManager = PlayerManager::GetInstance();

	// 変身中または変身要求が保留中は選択を無効化する
	if(playerManager->IsTransforming() || playerManager->IsTransformRequest())
	{
		_padInput5Active = false;
		return true;
	}

	// タヌキ表示中のみ選択可能
	ModeGame* mgCheck = nullptr;
	if(_owner)
	{
		mgCheck = StCas<ModeGame*>(_owner);
	}
	if(!(mgCheck && mgCheck->IsShowingTanuki()))
	{
		_padInput5Active = false;

		//// 人間表示時は PAD_INPUT_4 を押すことで UI 経由で即時タヌキへ戻す要求を出せるようにする
		//if(playerManager->GetPlayerState() == PlayerManager::PlayerState::HUMAN && (trg & PAD_INPUT_4))
		//{
		//	playerManager->RequestTransformToTanuki();
		//	return true;
		//}

		//// モノ表示時は PAD_INPUT_3 を押すことでタヌキへ戻す要求を出せるようにする
		//if(playerManager->GetPlayerState() == PlayerManager::PlayerState::MONO && (trg & PAD_INPUT_3))
		//{
		//	playerManager->RequestTransformToTanuki();
		//	return true;
		//}

		// 表示がタヌキでないなら以降の入力は無視
		return true;
	}

	// タヌキに戻った時、選択状態を自動的に有効化
	if(mgCheck && mgCheck->IsShowingTanuki() && !_padInput5Active)
	{
		_padInput5Active = true;
		_select = Select::TANUBITO;
	}

	return true;
}

bool HenshinUi::Render()
{
	base::Render();

	// オーナーが変身中かどうかをチェックし、変身中は選択表示を無効化する
	ModeGame* mg = nullptr;
	if(_owner)
	{
		mg = StCas<ModeGame*>(_owner);
	}
	
	// まきものの所持数をチェック
	bool hasMakimono = false;
	if(mg)
	{
		PlayerBase* currentPlayer = PlayerForm::GetInstance()->GetPlayer();
		hasMakimono = (currentPlayer != nullptr) && (currentPlayer->GetMakimonoCount() > 0);
	}

	// タヌビト画像
	if(_select == Select::TANUBITO)
	{
		const int handle = hasMakimono ? _handleMain : _handleNotCg;
		if(_handleMain != -1)
		{
			DrawGraph(henshin::HENSHIN_X, henshin::HENSHIN_Y, handle, TRUE);
		}
	}

	if(_showPlayerUi && hasMakimono)
	{
		if(_handleTanubito != -1)
		{
			DrawGraph(henshin::TANUBITO_X, henshin::TANUBITO_Y, _handleTanubito, TRUE);
		}
	}
	if(_showPlayerMonoUi && hasMakimono)
	{
		if(_handleMono != -1)
		{
			DrawGraph(henshin::TANUMONO_X, henshin::TANUMONO_Y, _handleMono, TRUE);
		}
	}

	return true;
}