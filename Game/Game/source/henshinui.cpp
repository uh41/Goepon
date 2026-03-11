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

	// タヌキに戻った時、選択状態を自動的に有効化（TANUBITO を選択状態にする）
	if(mgCheck && mgCheck->IsShowingTanuki() && !_padInput5Active)
	{
		_padInput5Active = true;
		_select = Select::TANUBITO;
	}

	// 最優先: PAD_INPUT_4 を押したら UI を開かずにすぐ TANUBITO を選択（変身要求を送る）
	// PAD_INPUT_5 の処理より前に配置することで即座に反応させる
	if(trg & PAD_INPUT_4)
	{
		if(_owner)
		{
			ModeGame* mg2 = StCas<ModeGame*>(_owner);
			if(mg2)
			{
				// 変身中/保留中なら無視
				if(!(mg2->IsTransforming() || mg2->IsTransformRequested()))
				{
					// UI が開いている場合は選択に応じて変身
					if(_padInput5Active)
					{
						if(_select == Select::TANUBITO)
						{
							mg2->RequestTransformToHuman();	// タヌキ -> 人間（アニメあり）の要求
						}
						else if(_select == Select::TANUMONO)
						{
							mg2->RequestTransformToMono();	// タヌキ -> モノ（巻物消費）の要求
						}
						_padInput5Active = false;
					}
					else
					{
						// UI が開いていない場合は直接 TANUBITO 変身
						_padInput5Active = true;
						_select = Select::TANUBITO;
						mg2->RequestTransformToHuman();
					}
					return true;
				}
			}
		}
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

		if(gGlobal._soundServer)
		{
			auto ui = gGlobal._soundServer->Get("63"); // "63" は UI_Henshin_pon
			if(ui)
			{
				ui->Play();
			}
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
	// 選択時は大きく、非選択時は小さく表示するためのスケール
	float selectScele = 1.6f; // 選択時の拡大率（大きく）
	float normalScele = 0.6f; // 非選択時の縮小率（小さく）

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

		// 選択時は大きい位置（巻物中央寄せ）、非選択時は小さいアイコン位置
		int x, y;
		if(effectivePad5 && _select == Select::TANUBITO)
		{
			// 大きく表示するときは巻物中心付近に表示（調整可）
			x = henshin::MAKIMONO_X + 48 - drawW / 2;
			y = henshin::MAKIMONO_Y + 36 - drawH / 2;
		}
		else
		{
			// 小アイコンは既定の位置
			x = henshin::TANUBITO_X - drawW / 2;
			y = henshin::TANUBITO_Y - drawH / 2;
		}

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

		// 選択時は大きい位置（巻物中央寄せ）、非選択時は小さいアイコン位置。
		// 非選択時はヒトの小アイコンと被らないよう右に少しずらす。
		int x, y;
		if(effectivePad5 && _select == Select::TANUMONO)
		{
			x = henshin::MAKIMONO_X + 48 - drawW / 2;
			y = henshin::MAKIMONO_Y + 36 - drawH / 2;
		}
		else
		{
			x = henshin::TANUMONO_X - drawW / 2 + 20; // ずらし量は必要に応じて調整
			y = henshin::TANUMONO_Y - drawH / 2;
		}

		DrawExtendGraph(x, y, x + drawW, y + drawH, _handleMono, TRUE);

		if(effectivePad5 && _select == Select::TANUMONO)
		{
			unsigned int color = GetColor(255, 255, 0); // 黄色
			DrawBox(x - 6, y - 6, x + drawW + 6, y + drawH + 6, color, FALSE);
		}
	}

	return true;
}