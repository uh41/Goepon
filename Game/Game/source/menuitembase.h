/*********************************************************************/
// * \file   menuitembase.h
// * \brief  デバックメニュークラス
// *
// * \author 鈴木裕稀
/*********************************************************************/

#pragma once
#include "appframe.h"
//#include "modeeffekseer.h"
#include "modegame.h"

#ifdef _DEBUG
#include <crtdbg.h>
#define NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
#else
#define NEW new
#endif

class MenuItemBase
{
public:
	MenuItemBase(void* param, std::string text);
	virtual ~MenuItemBase();

	// 項目を決定したらこの関数が呼ばれる
	// return int : 0 = メニュー継続, 1 = メニュー終了
	virtual int Selected();

	// この項目がカメラ操作切替項目かどうか（ModeMenu が判定するため）
	virtual bool IsCameraControlItem() { return false; }

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
		ModeGame* game = StCas<ModeGame*>(_param);
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
		ModeGame* game = StCas<ModeGame*>(_param);
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
		ModeGame* game = StCas<ModeGame*>(_param);
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
		ModeGame* game = StCas<ModeGame*>(_param);
		if(!game) return 1;
		// 既に起動済みなら停止（モードを検索して削除予約）
		if(game->GetEffekseerLaunched())
		{
			ModeBase* existing = ModeServer::GetInstance()->Get("effectsample");
			if(existing)
			{
				ModeServer::GetInstance()->Del(existing);
				game->SetEffekseerLaunched(false);
			}
			else
			{
				// フラグが立っているがモードが見つからない場合はフラグをクリアする
				game->SetEffekseerLaunched(false);
			}
			return 1; // メニューを閉じる
		}
		// ModeGameより上のレイヤーに登録する
		//ModeServer::GetInstance()->Add(NEW ModeEffekseer(), 100, "effectsample");
		// フラグを立ててメニューを閉じる
		game->SetEffekseerLaunched(true);
		return 1; // メニューを閉じる
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
		ModeGame* game = StCas<ModeGame*>(_param);
		game->SetDebugViewShadowMap(!game->GetDebugViewShadowMap());
		return 0;
	}
};

// カメラ操作モード切替メニュー項目（メニューを開いている間のみカメラ操作を有効にする）
class MenuItemCameraControlMode : public MenuItemBase
{
public:
	MenuItemCameraControlMode(void* param, std::string text) : MenuItemBase(param, text) {}
	virtual int Selected()
	{
		ModeGame* game = StCas<ModeGame*>(_param);
		if(!game)
		{
			return 0;
		}
		// カメラが未編集状態なら開始してメニューを閉じる
		if(!game->GetCameraControlMode())
		{
			game->StartCameraControlAndSave();
			return 1; // メニューを閉じる（編集モードへ）
		}
		else
		{
			// 既に編集中なら、選択で保存状態に戻す（メニューは継続）
			game->EndCameraControlAndRestore();
			return 0; // メニュー継続
		}
	}
	virtual bool IsCameraControlItem() override { return true; }
};

class MenuItemViewFps : public MenuItemBase
{
public:
	MenuItemViewFps(void* param, std::string text) : MenuItemBase(param, text) {}
	virtual ~MenuItemViewFps() {}

	// 項目を決定したらこの関数が呼ばれる
	// return int : 0 = メニュー継続, 1 = メニュー終了
	virtual int Selected()
	{
		ModeGame* game = StCas<ModeGame*>(_param);
		if(!game) return 0;

		// FPS表示の切り替え
		game->SetDebugViewFps(!game->GetDebugViewFps());

		// FrameRateControllerに設定を反映
		auto* app = ApplicationMain::GetInstance();
		if(app && app->GetFrameRateController())
		{
			app->GetFrameRateController()->EnableFpsDisplay(game->GetDebugViewFps());
		}

		return 0; // メニュー継続
	}

};

