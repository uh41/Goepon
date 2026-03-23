#include "henshinui.h"
#include "appframe.h"
#include "applicationglobal.h"
#include "modegame.h"

HenshinUi::HenshinUi()
{
	_handle = -1;
	_handleTanubito = -1;
	_handleMono = -1;
	_select = Select::TANUBITO;
	_padInput5Active = true;  // 初期状態で選択状態を有効化
	_owner = nullptr;
	_showPlayerUi = false;
	_showPlayerMonoUi = false;
	Initialize();
}

bool HenshinUi::Initialize()
{
	base::Initialize();
	_handle = LoadGraph(ui::UI_Hito);
	_handleTanubito = LoadGraph(ui::UI_Tanubito_no);
	_handleMono = LoadGraph(ui::UI_Mono);
	return true;
}

bool HenshinUi::Terminate()
{
	base::Terminate();

	if(_handle != -1)
	{
		DeleteGraph(_handle);
		_handle = -1;
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

	// オーナー（ModeGame）を参照して変身中かどうかを確認
	ModeGame* mg = nullptr;
	if(_owner)
	{
		mg = StCas<ModeGame*>(_owner);
	}

	// 変身中または変身要求が保留中は選択を無効化する（開閉/切替/決定 すべて無効）
	if(mg && (mg->IsTransforming() || mg->IsTransformRequested()))
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

		// 人間表示時は PAD_INPUT_4 を押すことで UI 経由で即時タヌキへ戻す要求を出せるようにする
		// （ModeGame 側でモノ表示時は無視されるため安全）
		if(mgCheck && !mgCheck->IsShowingMono() && (trg & PAD_INPUT_4))
		{
			mgCheck->RequestReturnToTanukiFromHuman();
			return true;
		}

		if(mgCheck && mgCheck->IsShowingMono() && (trg & PAD_INPUT_3))
		{
			mgCheck->RequestReturnToTanukiFromHuman(); // 既存の戻す要求を利用
			return true;
		}

		// 表示がタヌキでないなら以降の入力は無視
		return true;
	}

	// タヌキに戻った時、選択状態を自動的に有効化（TANUBITO を選択状態にする）
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
	// 表示側も「変身要求保留中」を考慮して選択表示を抑止
	bool effectivePad5 = _padInput5Active && !(mg && (mg->IsTransforming() || mg->IsTransformRequested()));

	// タヌビト画像
	if(_select == Select::TANUBITO)
	{
		if(_handle != -1)
		{
			DrawGraph(henshin::HENSHIN_X, henshin::HENSHIN_Y, _handle, TRUE);
		}
	}

	if(_showPlayerUi)
	{
		if(_handleTanubito != -1)
		{
			DrawGraph(henshin::TANUBITO_X, henshin::TANUBITO_Y, _handleTanubito, TRUE);
		}
	}
	if(_showPlayerMonoUi)
	{
		if(_handleMono != -1)
		{
			DrawGraph(henshin::TANUMONO_X, henshin::TANUMONO_Y, _handleMono, TRUE);
		}
	}

	return true;
}