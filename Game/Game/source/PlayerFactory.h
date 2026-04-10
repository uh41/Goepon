/*********************************************************************/
// * \file   playerfactory.cpp
// * \brief  プレイヤー工場クラス
// *
// * \author 鈴木裕稀
/*********************************************************************/

#pragma once
#include "appframe.h"
#include "playerbase.h"

class PlayerTanuki;
class Player;
class PlayerMono;

class PlayerFactory
{
public:
	using Creator = at::fupc<PlayerBase>; // プレイヤー生成関数の型
	using PlayerType = PlayerBase::PlayerType; // プレイヤーの種類を表す列挙型

	static bool Initialize();// 初期化
	static bool Terminate(); // 終了処理

	static PlayerBase* GetTanukiPlayer(); // タヌキプレイヤーを取得
	static PlayerBase* GetHumanPlayer(); // 人間プレイヤーを取得
	static PlayerBase* GetMonoPlayer(); // モノプレイヤーを取得
	static PlayerBase* GetPlayer(PlayerType type); // プレイヤーを取得

	static void RegisterType(PlayerType type, Creator creator); // プレイヤーの種類と生成関数を登録する関数

private:
	static at::upc<PlayerTanuki> _tanukiPlayer; // タヌキプレイヤーのインスタンス
	static at::upc<Player> _humanPlayer; // 人間プレイヤーのインスタンス
	static at::upc<PlayerMono> _monoPlayer; // モノプレイヤーのインスタンス

	static at::mtt<PlayerType, Creator>& GetRegistry(); // プレイヤーの種類と生成関数のマップを取得する関数
	static at::upc<PlayerBase> CreatePlayer(PlayerType type); // プレイヤーを生成する関数
};

