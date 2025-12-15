/*********************************************************************/
// * \file   modegameobject.cpp
// * \brief  モードゲームクラス(オブジェクト初期化)
// *
// * \author 鈴木裕稀
// * \date   2025/12/15
// * \作業内容: 新規作成 鈴木裕稀　2025/12/15
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
	_chara.emplace_back(_player);
	_playerBase.emplace_back(_player);
	
	// タヌキプレイヤー初期化
	_playerTanuki = std::make_shared<PlayerTanuki>();
	_chara.emplace_back(_playerTanuki);
	_playerBase.emplace_back(_playerTanuki);

	// 敵初期化
	_enemy.emplace_back(std::make_shared<Enemy>());
	_chara.emplace_back(_enemy.back());

	return true;
}
