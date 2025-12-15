/*********************************************************************/
// * \file   menuitembase.h
// * \brief  デバックメニュークラス
// *
// * \author 鈴木裕稀
// * \date   2025/12/15
// * \作業内容: 新規作成 鈴木裕稀　2025/12/15
/*********************************************************************/

#pragma once
#include "appframe.h"
#include "modeeffekseer.h"
#include "modegame.h"

class MenuItemBase
{
public:
	MenuItemBase(void* param, std::string text);
	virtual ~MenuItemBase();

	// 項目を決定したらこの関数が呼ばれる
	// return int : 0 = メニュー継続, 1 = メニュー終了
	virtual int Selected();

	void* _param;      // 項目に関連付けるパラメータ
	std::string _text; // メニューに表示するテキスト
};

// メニュー項目
class MenuItemViewCollision : public MenuItemBase
{
public:
	MenuItemViewCollision(void* param, std::string text): MenuItemBase(param, text) {}
	// 項目を決定したらこの関数が呼ばれる
	// return int : 0 = メニュー継続, 1 = メニュー終了
	virtual int Selected()
	{
		ModeGame* game = static_cast<ModeGame*>(_param);
		game->SetDebugViewCollsion(!game->GetDebugViewCollision());
		return 1;
	}
};

class MenuItemUseCollision : public MenuItemBase
{
public:
	MenuItemUseCollision(void* param, std::string text) : MenuItemBase(param, text) {}
	// 項目を決定したらこの関数が呼ばれる
	// return int : 0 = メニュー継続, 1 = メニュー終了
	virtual int Selected()
	{
		ModeGame* game = static_cast<ModeGame*>(_param);
		game->SetDebugUseCollision(!game->GetDebugUseCollision());
		return 1;
	}
};

class MenuItemViewCameraInfo : public MenuItemBase
{
public:
	MenuItemViewCameraInfo(void* param, std::string text) : MenuItemBase(param, text) {}
	// 項目を決定したらこの関数が呼ばれる
	// return int : 0 = メニュー継続, 1 = メニュー終了
	virtual int Selected()
	{
		ModeGame* game = static_cast<ModeGame*>(_param);
		game->SetDebugViewCameraInfo(!game->GetDebugViewCameraInfo());
		return 0;
	}
};

class MenuItemLaunchEffekseer : public MenuItemBase
{
public:
	MenuItemLaunchEffekseer(void* param, std::string text) : MenuItemBase(param, text) {}
	// 項目を決定したらこの関数が呼ばれる
	// return int : 0 = メニュー継続, 1 = メニュー終了
	virtual int Selected()
	{
		// ModeGameより上のレイヤーに登録する
		ModeServer::GetInstance()->Add(new ModeEffekseer(), 100, "effectsample");
		return 0;
	}
};

class MenuItemViewShadowMap : public MenuItemBase
{
public:
	MenuItemViewShadowMap(void* param, std::string text) : MenuItemBase(param, text) {}
	// 項目を決定したらこの関数が呼ばれる
	// return int : 0 = メニュー継続, 1 = メニュー終了
	virtual int Selected()
	{
		ModeGame* game = static_cast<ModeGame*>(_param);
		game->SetDebugViewShadowMap(!game->GetDebugViewShadowMap());
		return 0;
	}
};
