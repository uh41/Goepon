/*********************************************************************/
// * \file   playerfactory.cpp
// * \brief  プレイヤー工場クラス
// *
// * \author 鈴木裕稀
/*********************************************************************/

#include "PlayerFactory.h"
#include "playertanuki.h"
#include "player.h"
#include "playermono.h"

// 静的メンバの初期化
at::upc<PlayerTanuki> PlayerFactory::_tanukiPlayer = nullptr;
at::upc<Player> PlayerFactory::_humanPlayer = nullptr;
at::upc<PlayerMono> PlayerFactory::_monoPlayer = nullptr;

at::mtt<PlayerBase::PlayerType, PlayerFactory::Creator>& PlayerFactory::GetRegistry()
{
	static at::mtt<PlayerBase::PlayerType, Creator> registry; // プレイヤーの種類と生成関数のマップ
	return registry;
}

bool PlayerFactory::Initialize()
{

	RegisterType(PlayerType::TANUKI, []()// タヌキプレイヤーを登録
		{
			return std::make_unique<PlayerTanuki>();
		});

	RegisterType(PlayerType::HUMAN, []()// 人間プレイヤーを登録
		{
			return std::make_unique<Player>();
		});

	RegisterType(PlayerType::MONO, []()// モノプレイヤーを登録
		{
			return std::make_unique<PlayerMono>();
		});

	// タヌキプレイヤーの生成と初期化
	auto tanuki = CreatePlayer(PlayerType::TANUKI);
	if(!tanuki)
	{
		return false;
	}
	_tanukiPlayer = at::upc<PlayerTanuki>(static_cast<PlayerTanuki*>(tanuki.release()));
	if(!_tanukiPlayer->Initialize())
	{
		return false;
	}

	// 人間プレイヤーの生成と初期化
	auto human = CreatePlayer(PlayerType::HUMAN);
	if(!human)
	{
		return false;
	}
	_humanPlayer = at::upc<Player>(static_cast<Player*>(human.release()));
	if(!_humanPlayer->Initialize())
	{
		return false;
	}

	// モノプレイヤーの生成と初期化
	auto mono = CreatePlayer(PlayerType::MONO);
	if(!mono)
	{
		return false;
	}
	_monoPlayer = at::upc<PlayerMono>(static_cast<PlayerMono*>(mono.release()));
	if(!_monoPlayer->Initialize())
	{
		return false;
	}

	return true;
}

at::upc<PlayerBase> PlayerFactory::CreatePlayer(PlayerType type)
{
	auto it = GetRegistry().find(type);
	if(it != GetRegistry().end())
	{
		return it->second(); // 登録された生成関数を呼び出してプレイヤーを生成
	}
	return nullptr;
}

PlayerBase* PlayerFactory::GetTanukiPlayer()
{
	return _tanukiPlayer.get();
}

PlayerBase* PlayerFactory::GetHumanPlayer()
{
	return _humanPlayer.get();
}

PlayerBase* PlayerFactory::GetMonoPlayer()
{
	return _monoPlayer.get();
}

PlayerBase* PlayerFactory::GetPlayer(PlayerType type)
{
	switch(type)
	{
		case PlayerType::TANUKI:
		{
			return GetTanukiPlayer();
		}
		case PlayerType::HUMAN:
		{
			return GetHumanPlayer();
		}
		case PlayerType::MONO:
		{
			return GetMonoPlayer();
		}
		default:
		{
			return nullptr;
		}
	}
}

void PlayerFactory::RegisterType(PlayerType type, Creator creator)
{
	GetRegistry()[type] = creator; // プレイヤーの種類と生成関数をマップに登録
}

bool PlayerFactory::Terminate()
{
	if(_humanPlayer)
	{
		_humanPlayer->Terminate();
		_humanPlayer.reset();
	}

	if(_tanukiPlayer)
	{
		_tanukiPlayer->Terminate();
		_tanukiPlayer.reset();
	}

	if(_monoPlayer)
	{
		_monoPlayer->Terminate();
		_monoPlayer.reset();
	}

	return true;
}