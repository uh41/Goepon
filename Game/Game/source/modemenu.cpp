#include "modemenu.h"
#include "appframe.h"
#include "applicationmain.h"
#include "applicationglobal.h"
#include "menuitembase.h"

// 初期化
bool ModeMenu::Initialize()
{
	if(!base::Initialize()) { return false; }

	_cur_pos = 0;
	_cur_anim_cnt = 0;

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
		_cur_pos--;
		_cur_anim_cnt = 0;
	}
	if(trg & PAD_INPUT_DOWN)
	{
		_cur_pos++;
		_cur_anim_cnt = 0;
	}

	// カーソル位置を上下ループ
	int itemNum = (int)_items.size();
	_cur_pos = (_cur_pos + itemNum) % itemNum;

	// 決定でアイテムのSelected()を呼ぶ
	if(trg & PAD_INPUT_1)
	{
		int ret = _items[_cur_pos]->Selected();
		if(ret == 1)
		{
			// メニューを閉じる
			close = true;
		}
	}

	// メニューを閉じる
	if(close)
	{
		ModeServer::GetInstance()->Del(this);
	}

	_cur_anim_cnt++;

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
	DrawGraph(x + 4 + ((_cur_anim_cnt / 6) % 4) * 4, y + start_y + fontPitch * _cur_pos, gGlobal._cg_cursor, TRUE);

	return true;
}
