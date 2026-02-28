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
	_handle = ResourceServer::LoadGraph(img::UI_Makimono);
	_handleTanubito = ResourceServer::LoadGraph(img::UI_Tanubito);
	_handleMono = ResourceServer::LoadGraph(img::UI_Tanumono);
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

	// 変身中または変身要求が保留中は選択を無効化する（開閉/左右/決定 すべて無効）
	if(mg && (mg->IsTransforming() || mg->IsTransformRequested()))
	{
		_padInput5Active = false;
		return true;
	}

	// ホールドのみで選択開始する（押し始めは無視）
	bool hold5 = ((key & PAD_INPUT_5) != 0) && !(mg && (mg->IsTransforming() || mg->IsTransformRequested()));    // ホールド（変身中は無効）

	// hold が有効な間だけ選択状態を維持する（ホールドを離したら自動的に閉じる）
	if(hold5)
	{
		ModeGame* mgCheck = nullptr;
		if(_owner)
		{
			mgCheck = StCas<ModeGame*>(_owner);
		}
		// タヌキ表示中のみ選択開始可能
		if(mgCheck && mgCheck->IsShowingTanuki())
		{
			// ホールド開始の立ち上がりで初期選択を設定
			if(!_padInput5Active)
			{
				_padInput5Active = true;
				_select = Select::TANUBITO;
			}
		}
		else
		{
			// 表示がタヌキでないなら選択しない
			_padInput5Active = false;
		}
	}
	else
	{
		// ホールドを離したら必ず選択を閉じる
		_padInput5Active = false;
	}

	// 5Aが有効な状態で左右入力があった場合、選択肢を切り替える
	if(_padInput5Active)
	{
		// 選択肢のうち UI に表示している項目のみ移動対象にする
		// (Select::TANUKI は UI 表示がないため選択できないようにする)
		int firstSelectable = StCas<int>(Select::TANUBITO); // 最小選択インデックス
		int lastSelectable = StCas<int>(Select::TANUMONO); // 最大選択インデックス
		int idx = StCas<int>(_select); // 現在の選択インデックスを取得

		// 左で前へ、右で次へ。ただし範囲外へは出ないようにクランプする
		if(trg & PAD_INPUT_LEFT)
		{
			if(idx > firstSelectable) idx--;
		}
		else if(trg & PAD_INPUT_RIGHT)
		{
			if(idx < lastSelectable) idx++;
		}

		_select = StCas<Select>(idx); // 新しい選択肢をセット

		if(trg & PAD_INPUT_4)
		{
			if(_owner)
			{
				ModeGame* mg = StCas<ModeGame*>(_owner);
				if(mg)
				{
					// 念のため再確認：変身中なら無視
					if(mg->IsTransforming() || mg->IsTransformRequested())
					{
						_padInput5Active = false;
						return true;
					}

					// 直接変身処理を呼ばず、要求 API を呼ぶ（ModeGame 側で処理・同期される）
					if(_select == Select::TANUBITO)
					{
						mg->RequestTransformToHuman();	// タヌキ -> 人間（アニメあり）の要求
					}
					else if(_select == Select::TANUMONO)
					{
						mg->RequestTransformToMono();	// タヌキ -> モノ（巻物消費）の要求
					}
				}
			}
			_padInput5Active = false; // 変身が決定したら選択状態をリセット
		}
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