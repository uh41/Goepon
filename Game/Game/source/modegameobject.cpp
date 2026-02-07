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
	//// マップ初期化
	//_map = std::make_shared<Map>();
	//_object.emplace_back(_map);

	// プレイヤー初期化
	_player = std::make_shared<Player>();
	_playerBase.emplace_back(_player);
	_playerTanuki = std::make_shared<PlayerTanuki>();
	_playerBase.emplace_back(_playerTanuki);
	_playerMono = std::make_shared<PlayerMono>();
	_playerBase.emplace_back(_playerMono);

	// 宝箱初期化
	_treasure = std::make_shared<Treasure>();
	_object.emplace_back(_treasure);

	// ゴール初期化
	_goal = std::make_shared<Goal>();
	_object.emplace_back(_goal);

	// ui初期化
	_uiHp = std::make_shared<UiHp>();
	_uiHp->SetPlayer(_player.get());
	_uiBase.emplace_back(_uiHp);

	// エフェクト初期化
	_treasureEffect = std::make_shared<TreasureEffect>();
	_effectBase.emplace_back(_treasureEffect);

	return true;
}

// 影の初期化
bool ModeGame::ShadowInitialize()
{
	auto charaShadow = std::make_shared<CharaShadow>();
	// プレイヤーに対するシャドウ
	charaShadow->SetTargetChara([this]() -> CharaBase*
		{
			if(_showMonoPlayer)
			{
				return _playerMono.get();
			}
			else if(_bShowTanuki)
			{
				return _playerTanuki.get();
			}
			else
			{
				return _player.get();
			}
		});
	_charaShadow.emplace_back(charaShadow);

	// 敵のシャドウは実際に処理されるコンテナ（_enemyBase）を参照するように変更
	for(auto& eb : _enemyBase)
	{
		if(!eb)
		{
			continue;
		}
		auto shadow = std::make_shared<CharaShadow>();
		// eb をキャプチャして EnemyBase* を返すラムダを渡す
		shadow->SetTargetChara([eb]() -> CharaBase* { return static_cast<CharaBase*>(eb.get()); });
		_charaShadow.emplace_back(shadow);
	}

	return true;
}

// プレイヤー変身処理
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

		// Effekseer のエフェクトを再生（タヌキ->人間 変身完了時）
		if(_henshineffectHandle != -1)
		{
			// プレイヤー位置にエフェクトを出す（必要ならオフセットを調整）
			EffekseerManager::GetInstance()->PlayEffect3DPos(_henshineffectHandle, _player->GetPos());
		}

		_player->Process();
		return true;
	}

	// （以下はそのまま）
	int trg = ApplicationMain::GetInstance()->GetTrg();

	// PAD_INPUT_3: タヌキ <-> モノ 切替
	if(trg & PAD_INPUT_3)
	{
		// 無効化：現在「プレイヤー（人間）」表示中なら PAD_INPUT_3 は何もしない
		// （プレイヤー＝_bShowTanuki==false && _showMonoPlayer==false）
		if(!_bShowTanuki && !_showMonoPlayer)
		{
			// プレイヤー（人間）時は無効化、何もしない
		}
		else
		{
			// タヌキ表示中ならモノに切り替え
			if(_bShowTanuki)
			{
				_bShowTanuki = false;
				_showMonoPlayer = true;

				_playerMono->SetPos(_playerTanuki->GetPos());
				_playerMono->SetDir(_playerTanuki->GetDir());
				_playerMono->_status = CharaBase::STATUS::WAIT;
				_playerMono->PlayAnimation("idle_kari", true);
				_playerMono->Process();
				return true;
			}
			else if(_showMonoPlayer)
			{
				// フラグ更新
				_showMonoPlayer = false;
				_bShowTanuki = true;
				_playerTanuki->SetPos(_playerMono->GetPos());
				_playerTanuki->SetDir(_playerMono->GetDir());
				_playerTanuki->_status = CharaBase::STATUS::WAIT;
				_playerTanuki->PlayAnimation("goepon_idle", true);
				_playerTanuki->Process();
				return true;
			}
		}
	}

	// PAD_INPUT_4: タヌキ -> 人間 の変身（無効化：モノ表示時は変身不可）
	if(!_bTransCancel)
	{
		if(trg & PAD_INPUT_4)
		{
			// 無効化：現在「モノ」表示中なら PAD_INPUT_4 を無効化する
			if(_showMonoPlayer)
			{
				// モノ時は変身不可、何もしない
			}
			else
			{
				if(_bShowTanuki)
				{
					_transformAnimId = _playerTanuki->PlayAnimation("gomepon_hensin", false);
					_isTransformingToHuman = true;

					// 変身中はタヌキのまま処理
					_playerTanuki->Process();
					return true;
				}
				else
				{
					_bShowTanuki = true;
					_playerTanuki->SetPos(_player->GetPos());
					_playerTanuki->SetDir(_player->GetDir());
					_playerTanuki->_status = CharaBase::STATUS::WAIT;
					_playerTanuki->PlayAnimation("goepon_idle", true);
				}
			}
		}
	}



	// プレイヤーの処理（現在表示中のプレイヤーのみ）
	if(_bShowTanuki)
	{
		_playerTanuki->Process();
	}
	else if(_showMonoPlayer)
	{
		_playerMono->Process();
	}
	else
	{
		_player->Process();
	}

	return true;
}

// オブジェクト処理
bool ModeGame::ObjectProcess()
{
	// オブジェクト処理
	for(auto& object : _object)
	{
		object->Process();
	}

	// キャラ処理（生存しているもののみ）
	for(auto& chara : _chara)
	{
		if(chara->IsAlive())
		{
			chara->Process();
		}
	}

	// 敵（追跡/移動はここで実行される）
	for(auto& enemy : _enemyBase)
	{
		if(enemy->IsAlive())
		{
			enemy->Process();
		}
	}


	// キャラクターの影処理
	for(auto& shadow : _charaShadow)
	{
		if(shadow)
		{
			shadow->Process();
		}
	}

	// UI処理
	for(auto& ui_base : _uiBase)
	{
		ui_base->Process();
	}

	// エフェクト処理
	for(auto& effect_base : _effectBase)
	{
		effect_base->Process();
	}
	return true;
}

// BGMチェンジ処理
bool ModeGame::ChangeBGM()
{
	bool isChase = false;
	if(_enemySensor)
	{
		isChase = _enemySensor->IsChasing();
	}

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