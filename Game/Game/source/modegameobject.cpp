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

	// 宝箱初期化
	//_object.emplace_back(std::make_shared<Treasure>());
	_object.emplace_back(std::make_shared<Treasure>());
	_treasure = std::make_shared<Treasure>();

	// ui初期化
	_uiHp = std::make_shared<UiHp>();
	_uiHp->SetPlayer(_player.get());
	_uiBase.emplace_back(_uiHp);

	
	
	return true;
}

bool ModeGame::ShadowInitialize()
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

bool ModeGame::PlayerTransform()
{
	// 変身アニメ中の監視（タヌキ -> 人間）
	if(_isTransformingToHuman)
	{
		// まだ再生中なら、タヌキ表示のまま継続
		if(_transformAnimId != -1 && AnimationManager::GetInstance()->IsPlaying(_transformAnimId))
		{
			_playerTanuki->Process();
			return true;
		}

		// 再生が終わったので、人間へ切り替え（ここで座標同期）
		_isTransformingToHuman = false;
		_transformAnimId = -1;

		_bShowTanuki = false;
		_player->SetPos(_playerTanuki->GetPos());
		_player->SetDir(_playerTanuki->GetDir());

		_player->Process();
		return true;
	}

	int trg = ApplicationMain::GetInstance()->GetTrg();

	// タヌキプレイヤー表示切替
	if(trg & PAD_INPUT_4)
	{
		// いまタヌキ表示なら「タヌキ -> 人間」はアニメを見せたいので即切替しない
		if(_bShowTanuki)
		{
			_transformAnimId = _playerTanuki->PlayAnimation("hensin", false);
			_isTransformingToHuman = true;

			if(_soundServer)
			{
				_soundServer->Add(new soundserver::SoundItemOneShot("res/OneShot/7_01.mp3"));
			}

			// 変身中はタヌキのまま処理
			_playerTanuki->Process();
			return true;
		}
		else
		{

			// 人間 -> タヌキ（こちらは今まで通り即切替）
			_bShowTanuki = true;
			_playerTanuki->SetPos(_player->GetPos());
			_playerTanuki->SetDir(_player->GetDir());
			_playerTanuki->PlayAnimation("hensin", false);

			_playerTanuki->_status = CharaBase::STATUS::WAIT;
			_playerTanuki->PlayAnimation("taiki", true);
		}


		if(_soundServer)
		{
			_soundServer->Add(new soundserver::SoundItemOneShot("res/OneShot/7_01.mp3"));
		}

		// シャドウの追従キャラも切り替え
		if(!_charaShadow.empty())
		{
			auto& playerShadow = _charaShadow.front();
			if(playerShadow)
			{
				playerShadow->SetTargetChara(static_cast<PlayerBase*>(_playerTanuki.get()));
			}
		}
	}

	// プレイヤーの処理（現在表示中のプレイヤーのみ）
	if(_bShowTanuki)
	{
		_playerTanuki->Process();
	}
	else
	{
		_player->Process();
	}

	return true;
}

bool ModeGame::ObjectProcess()
{

	// キャラ処理（生存しているもののみ）
	for(auto& chara : _chara)
	{
		if(chara->IsAlive())
		{
			chara->Process();
		}
	}

	// エネミーの処理
	for(auto& enemy : _enemy)
	{
		if(enemy->IsAlive())
		{
			enemy->Process();
		}
	}

	// オブジェクト処理
	for(auto& object : _object)
	{
		object->Process();
	}

	/*for(auto& treasure : _treasure)
	{
		treasure->Process();
	}*/

	// UI処理
	for(auto& ui_base : _uiBase)
	{
		ui_base->Process();
	}
	return true;
}

bool ModeGame::ChangeBGM()
{
	bool isChase = false;
	//if(_enemySensor)
	//{
	//	isChase = _enemySensor->IsChasing();
	//}

	// BGMチェンジ処理
	if(!_isChengeBgm && isChase)
	{
		_bgmInitialize->Stop();
		_bgmChenge->Play();
		_isChengeBgm = true;
	}
	else if(_isChengeBgm && !isChase)
	{
		_bgmChenge->Stop();
		_bgmInitialize->Play();
		_isChengeBgm = false;
	}

	return true;
}