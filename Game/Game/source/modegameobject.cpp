#include "modegame.h"
#include "player.h"
#include "cube.h"
#include "map.h"

// オブジェクトの初期化
// オブジェクトの初期化
bool ModeGame::ObjectInitialize()
{
	// キューブ初期化
	_cube = std::make_shared<Cube>();
	_object.emplace_back(_cube);

	// マップ初期化
	_map = std::make_shared<Map>();
	_object.emplace_back(_map);

	// プレイヤー初期化
	_player = std::make_shared<Player>();
	_chara.emplace_back(_player);
	_player_base.emplace_back(_player);
	
	// タヌキプレイヤー初期化
	_player_tanuki = std::make_shared<PlayerTanuki>();
	_chara.emplace_back(_player_tanuki);
	_player_base.emplace_back(_player_tanuki);

	// 敵初期化
	_enemy.emplace_back(std::make_shared<Enemy>());
	_chara.emplace_back(_enemy.back());

	return true;
}
