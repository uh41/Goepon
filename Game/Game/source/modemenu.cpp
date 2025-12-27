/*********************************************************************/
// * \file   modemenu.cpp
// * \brief  モードメニュークラス
// *
// * \author 鈴木裕稀
// * \date   2025/12/15
// * \作業内容: 新規作成 鈴木裕稀　2025/12/15
/*********************************************************************/

#include "modemenu.h"
#include "appframe.h"
#include "applicationmain.h"
#include "applicationglobal.h"
#include "menuitembase.h"

// 初期化
bool ModeMenu::Initialize()
{
	if(!base::Initialize()) { return false; }

	_iCurPos = 0;
	_iCurAnimCnt = 0;
	_owner = nullptr;

	return true;
}

// 終了
bool ModeMenu::Terminate()
{
	base::Terminate();
	Clear();
	return true;
}

// クリア
void ModeMenu::Clear()
{
	for(auto* ite : _items)
	{
		delete ite;
	}
	_items.clear();
}

// 項目追加
void ModeMenu::Add(MenuItemBase* itemBase)
{
	_items.push_back(itemBase);
}

// 計算処理
bool ModeMenu::Process()
{
	base::Process();
	int key = ApplicationMain::GetInstance()->GetKey();
	int trg = ApplicationMain::GetInstance()->GetTrg();

	// このモードより下のレイヤーはProcess()を呼ばない
	ModeServer::GetInstance()->SkipProcessUnderLayer();

	// ESCキーでメニューを閉じる
	bool close = false;
	if(trg & PAD_INPUT_9)
	{
		close = true;
	}

	// 上下でカーソルを移動する
	if(trg & PAD_INPUT_UP)
	{
		_iCurPos--;
		_iCurAnimCnt = 0;
	}
	if(trg & PAD_INPUT_DOWN)
	{
		_iCurPos++;
		_iCurAnimCnt = 0;
	}

	// カーソル位置を上下ループ
	int itemNum = (int)_items.size();
	_iCurPos = (_iCurPos + itemNum) % itemNum;

	// 決定でアイテムのSelected()を呼ぶ
	if(trg & PAD_INPUT_1)
	{
		int ret = _items[_iCurPos]->Selected();
		MenuItemBase* sel = _items[_iCurPos];
		if(ret == 1)
		{
			// カメラ項目かつメニュー閉じの返り値なら、確実に Start してから閉じる
			if(sel->IsCameraControlItem())
			{
				ModeGame* owner = static_cast<ModeGame*>(_owner);
				if(owner && !owner->GetCameraControlMode())
				{
					owner->StartCameraControlAndSave();
				}
			}
			// メニューを閉じる
			close = true;
		}
		else
		{
			// ret == 0 のとき、カメラ項目なら Selected() likely ended camera control; ensure End is called
			if(sel->IsCameraControlItem())
			{
				ModeGame* owner = static_cast<ModeGame*>(_owner);
				if(owner && owner->GetCameraControlMode())
				{
					owner->EndCameraControlAndRestore();
				}
			}
		}
	}

	// メニューを閉じる
	if(close)
	{
		ModeServer::GetInstance()->Del(this);
	}

	_iCurAnimCnt++;

	return true;
}

// 描画処理
bool ModeMenu::Render()
{
	base::Render();

	// メニュー項目の確認
	int x = 128;
	int y = 128;
	int w = 0;
	int h = 0;
	int fontSize = 32;
	int fontPitch = fontSize + 8;
	SetFontSize(fontSize);
	for(auto* ite : _items)
	{
		int item_w = GetDrawStringWidth((ite)->_text.c_str(), static_cast<int>((ite)->_text.length()));
		if(w < item_w)
		{
			w = item_w;
		}
		h += fontPitch;
	}
	// カーソル、枠分のサイズ拡張
	w += 64 + 16;
	h += 16;

	// 下地の描画
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 128);
	DrawBox(x, y, x + w, y + h, GetColor(0, 0, 255), TRUE);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	DrawBox(x, y, x + w, y + h, GetColor(0, 0, 255), TRUE);

	// メニュー項目の描画
	int start_y = 16 / 2;
	h = 0;
	for(auto* ite : _items)
	{
		DrawString(x + 64, y + start_y + h, (ite)->_text.c_str(), GetColor(255, 0, 0));
		h += fontPitch;
	}
	// カーソルの描画
	// x座標はアニメーションする
	DrawGraph(x + 4 + ((_iCurAnimCnt / 6) % 4) * 4, y + start_y + fontPitch * _iCurPos, gGlobal._iCgCursor, TRUE);

	return true;
}
