/*********************************************************************/
// * \file   modegameobject.cpp
// * \brief  モードゲームクラス(オブジェクト初期化)
// *
// * \author 鈴木裕稀
// * \date   2025/12/15
// * \作業内容	: 新規作成 鈴木裕稀　2025/12/15
//				: UI HP追加	鈴木裕稀 2026/01/06
/*********************************************************************/

#include "modegame.h"
#include "player.h"
#include "cube.h"
#include "map.h"

// オブジェクトの初期化
bool ModeGame::ObjectInitialize()
{
	// キューブ初期化
	/*_cube = std::make_shared<Cube>();
	_object.emplace_back(_cube);*/

	// マップ初期化
	_map = std::make_shared<Map>();
	_object.emplace_back(_map);

	// プレイヤー初期化
	_player = std::make_shared<Player>();
	// プレイヤーは描画・処理を_playerBaseで管理するので_charaには追加しない
	_playerBase.emplace_back(_player);

	// タヌキプレイヤー初期化
	_playerTanuki = std::make_shared<PlayerTanuki>();
	// タヌキプレイヤーも_playerBaseで管理
	_playerBase.emplace_back(_playerTanuki);

	// 敵初期化
	_enemy.emplace_back(std::make_shared<Enemy>());
	_chara.emplace_back(_enemy.back());

	// ui初期化
	_uiHp = std::make_shared<UiHp>();
	_uiHp->SetPlayer(_player.get());
	_uiBase.emplace_back(_uiHp);

	// --- ここからシャドウ生成 ---
	// プレイヤー（通常）用シャドウ
	{
		auto charaShadow = std::make_shared<CharaShadow>();
		// 初期ターゲットはフラグに応じて設定
		if(_bShowTanuki)
		{
			charaShadow->SetTargetChara(_playerTanuki.get());
		}
		else
		{
			charaShadow->SetTargetChara(_player.get());
		}
		_charaShadow.emplace_back(charaShadow);


	}
	for(auto& c : _chara)
	{
		if(!c)
		{
			continue;
		}
		// プレイヤー（通常）とタヌキを除外
		if(c.get() == _player.get() || c.get() == _playerTanuki.get())
		{
			continue;
		}

		auto shadow = std::make_shared<CharaShadow>();
		// Scaleも調整可能
		shadow->SetTargetChara(c.get());
		_charaShadow.emplace_back(shadow);
	}
	
	return true;
}