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
	_padInput5Active = false;
	_owner = nullptr;
	Initialize();
}

bool HenshinUi::Initialize()
{
	base::Initialize();
	_handle = LoadGraph(img::UI_Makimono);
	_handleTanubito = LoadGraph(img::UI_Tanubito);
	_handleMono = LoadGraph(img::UI_Tanumono);
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
		if(mgCheck && (trg & PAD_INPUT_4))
		{
			mgCheck->RequestReturnToTanukiFromHuman();
			return true;
		}

		// 表示がタヌキでないなら以降の入力は無視
		return true;
	}

	// PAD_INPUT_5 の立ち上がり（トリガー）で選択を開始／切替する
	if(trg & PAD_INPUT_5)
	{
		// まだ選択中でなければ開いて最初は TANUBITO にする
		if(!_padInput5Active)
		{
			_padInput5Active = true;
			_select = Select::TANUBITO;
		}
		else
		{
			// 既に選択中なら TANUBITO <-> TANUMONO をトグルする
			if(_select == Select::TANUBITO)
			{
				_select = Select::TANUMONO;
			}
			else
			{
				_select = Select::TANUBITO;
			}
		}
	}

	// 選択中に PAD_INPUT_4 が押されたら変身要求を発行して選択を閉じる
	if(_padInput5Active && (trg & PAD_INPUT_4))
	{
		if(_owner)
		{
			ModeGame* mg2 = StCas<ModeGame*>(_owner);
			if(mg2)
			{
				// 念のため再確認：変身中なら無視
				if(mg2->IsTransforming() || mg2->IsTransformRequested())
				{
					_padInput5Active = false;
					return true;
				}

				// 選択に応じて変身要求を送る
				if(_select == Select::TANUBITO)
				{
					mg2->RequestTransformToHuman();	// タヌキ -> 人間（アニメあり）の要求
				}
				else if(_select == Select::TANUMONO)
				{
					mg2->RequestTransformToMono();	// タヌキ -> モノ（巻物消費）の要求
				}
			}
		}
		_padInput5Active = false; // 変身が決定したら選択状態をリセット
	}

	return true;
}



bool HenshinUi::Render()
{
	base::Render();

	if(_handle != -1)
	{
		DrawGraph(henshin::MAKIMONO_X, henshin::MAKIMONO_Y, _handle, TRUE);
	}

	// ここで早期リターンしない：タヌビト/モノ画像は常時表示する
	float selectScele = 1.25f;
	float normalScele = 1.0f;

	// オーナーが変身中かどうかをチェックし、変身中は選択表示を無効化する
	ModeGame* mg = nullptr;
	if(_owner)
	{
		mg = StCas<ModeGame*>(_owner);
	}
	// 表示側も「変身要求保留中」を考慮して選択表示を抑止
	bool effectivePad5 = _padInput5Active && !(mg && (mg->IsTransforming() || mg->IsTransformRequested()));

	// タヌビト画像
	if(_handleTanubito != -1)
	{
		int origW = 0;
		int origH = 0;
		GetGraphSize(_handleTanubito, &origW, &origH);

		float scale = normalScele;
		if(effectivePad5 && _select == Select::TANUBITO)
		{
			scale = selectScele;
		}

		int drawW = StCas<int>(origW * scale);
		int drawH = StCas<int>(origH * scale);
		int x = henshin::TANUBITO_X - drawW / 2;
		int y = henshin::TANUBITO_Y - drawH / 2;
		DrawExtendGraph(x, y, x + drawW, y + drawH, _handleTanubito, TRUE);

		if(effectivePad5 && _select == Select::TANUBITO)
		{
			unsigned int color = GetColor(255, 255, 0); // 黄色
			DrawBox(x - 6, y - 6, x + drawW + 6, y + drawH + 6, color, FALSE);
		}
	}

	// モノ画像
	if(_handleMono != -1)
	{
		int origW = 0;
		int origH = 0;
		GetGraphSize(_handleMono, &origW, &origH);

		float scale = normalScele;
		if(effectivePad5 && _select == Select::TANUMONO)
		{
			scale = selectScele;
		}

		int drawW = StCas<int>(origW * scale);
		int drawH = StCas<int>(origH * scale);
		int x = henshin::TANUMONO_X - drawW / 2;
		int y = henshin::TANUMONO_Y - drawH / 2;
		DrawExtendGraph(x, y, x + drawW, y + drawH, _handleMono, TRUE);

		if(effectivePad5 && _select == Select::TANUMONO)
		{
			unsigned int color = GetColor(255, 255, 0); // 黄色
			DrawBox(x - 6, y - 6, x + drawW + 6, y + drawH + 6, color, FALSE);
		}
	}

	return true;
}