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
	// カメラ初期化
	_camera = new Camera();
	_camera->Initialize();

	// マップ初期化
	_map = std::make_shared<Map>();
	_object.emplace_back(_map);

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

	// ui初期化
	_uiHp = std::make_shared<UiHp>();
	_uiHp->SetPlayer(_player.get());
	_uiBase.emplace_back(_uiHp);

	// エフェクト初期化
	_treasureEffect = std::make_shared<TreasureEffect>();
	_effectBase.emplace_back(_treasureEffect);
	_hensinEffect = std::make_shared<HensinEffect>();
	_effectBase.emplace_back(_hensinEffect);
	_walkEffect = std::make_shared<WalkEffect>();
	_effectBase.emplace_back(_walkEffect);
	_findEffect = std::make_shared<FindEffect>();
	_effectBase.emplace_back(_findEffect);

	// キャラ
	for(auto& chara : _chara)
	{
		chara->Initialize();
	}

	// プレイヤー
	for(auto& player_base : _playerBase)
	{
		player_base->Initialize();
	}

	// オブジェクトの初期化
	for(auto& object : _object)
	{
		object->Initialize();
	}

	// UI
	for(auto& ui_base : _uiBase)
	{
		ui_base->Initialize();
	}

	// エフェクト
	for(auto& effectBase : _effectBase)
	{
		effectBase->Initialize();
	}

	// シャドウ初期化
	for(auto& charaShadow : _charaShadow)
	{
		charaShadow->Initialize();
	}

	return true;
}

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

bool ModeGame::CameraInfoInitialize()
{
	// カメラをプレイヤー位置に合わせる（JSONでプレイヤー位置を読み込んだ直後に適用）
	if(_camera != nullptr)
	{
		// カメラの現在のオフセット（pos - target）を保存しておき、プレイヤーに合わせて再設定する
		vec::Vec3 camDelta = vec3::VSub(_camera->_vPos, _camera->_vTarget);

		// 初期表示プレイヤー（タヌキ／人間）に合わせる
		PlayerBase* startPlayer = nullptr;
		if(_bShowTanuki)
		{
			startPlayer = _playerTanuki.get();
		}
		else
		{
			startPlayer = _player.get();
		}

		if(startPlayer != nullptr)
		{
			// ターゲットはプレイヤーの高さを少し上げて注視する（元のカメラ設定に合わせる）
			vec::Vec3 target = vec3::VAdd(startPlayer->GetPos(), vec3::VGet(0.0f, 60.0f, 0.0f));
			_camera->_vTarget = target;
			_camera->_vPos = vec3::VAdd(target, camDelta);
		}
	}
	return true;
}

bool ModeGame::PlayerTransformToTanuki(bool player)
{
	// 変身アニメ開始（未開始時のみ）
	if(_transformAnimId == -1)
	{
		_transformAnimId = _playerTanuki->PlayAnimation("gomepon_hensin", false);
		if(player)
		{
			_isTransformToHuman = true;
		}
		else
		{
			_isTransformToMono = true;
		}
	}

	if(player)
	{
		if(_isTransformToHuman)
		{
			// アニメ再生中はタヌキ側だけ更新して待つ
			if(_transformAnimId != -1 && AnimationManager::GetInstance()->IsPlaying(_transformAnimId))
			{
				_playerTanuki->Process();
				return true;
			}

			// アニメ終了 → 人間へ切替
			_isTransformToHuman = false;
			_transformAnimId = -1;

			_bShowTanuki = false; // 人間表示に切替
			_showMonoPlayer = false;
			_player->SetPos(_playerTanuki->GetPos());
			_player->SetDir(_playerTanuki->GetDir());
			_hensinEffect->PlayEffect(_player->GetPos());

			// タヌキから人間への変身完了時に音波を発生
			for(auto& enemy : _enemyBase)
			{
				if(enemy->IsAlive())
				{
					enemy->GetSoundSensor()->TriggerSoundWave(_player->GetPos(), 500.0f, 10.0f);
					enemy->GetSoundSensor()->SetSoundLevel(5);
				}
			}
			_player->Process(); // 変身直後の一フレーム更新
			return true;
		}
	}
	else
	{
		if(_isTransformToMono)
		{
			// アニメ再生中はタヌキ側だけ更新して待つ
			if(_transformAnimId != -1 && AnimationManager::GetInstance()->IsPlaying(_transformAnimId))
			{
				_playerTanuki->Process();
				return true;
			}

			// アニメ終了 → モノへ切替
			_isTransformToMono = false;
			_transformAnimId = -1;

			_bShowTanuki = false;
			_showMonoPlayer = true;
			_playerMono->SetPos(_playerTanuki->GetPos());
			_playerMono->SetDir(_playerTanuki->GetDir());
			// タヌキから人間への変身完了時に音波を発生
			for(auto& enemy : _enemyBase)
			{
				if(enemy->IsAlive())
				{
					enemy->GetSoundSensor()->TriggerSoundWave(_playerMono->GetPos(), 500.0f, 10.0f);
					enemy->GetSoundSensor()->SetSoundLevel(5);
				}
			}
			_playerMono->Process(); // 変身直後の一フレーム更新
			_hensinEffect->PlayEffect(_playerMono->GetPos());
			return true;
		}
	}

	// 変身開始も変身中でもない場合は false を返し、呼び出し元で通常処理に進むようにする
	return false;
}

bool ModeGame::PlayerTransform()
{
	// AnimationManager の更新は呼び出し側で行われている前提（modegame.cpp）
	int trg = ApplicationMain::GetInstance()->GetTrg();

	// 変身進行中は入力に関係なく毎フレーム進行させる
	if(_isTransformToHuman || _isTransformToMono || _transformAnimId != -1)
	{
		if(_isTransformToHuman)
		{
			// true = 人間へ変身
			if(PlayerTransformToTanuki(true))
			{
				// 変身処理中または変身直後の1フレーム更新を行ったのでここで終了
				return true;
			}
		}
		else if(_isTransformToMono)
		{
			// false = モノへ変身
			if(PlayerTransformToTanuki(false))
			{
				return true;
			}
		}
	}

	// --- 以下、既存の入力処理（変更なし） ---
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
			// タヌキ表示中ならモノに切り替え（変身開始）
			if(_bShowTanuki)
			{
				if(PlayerTransformToTanuki(false))
				{
					return true;
				}
			}
			else if(_showMonoPlayer)
			{
				// モノ -> タヌキ は即時切替（アニメなし）
				_showMonoPlayer = false;
				_bShowTanuki = true;
				_playerTanuki->SetPos(_playerMono->GetPos());
				_playerTanuki->SetDir(_playerMono->GetDir());
				_playerTanuki->_status = CharaBase::STATUS::WAIT;
				_playerTanuki->PlayAnimation("goepon_idle", true);
				_playerTanuki->Process();
				_hensinEffect->PlayEffect(_playerTanuki->GetPos());
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
					if(PlayerTransformToTanuki(true))
					{
						return true;
					}
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

bool ModeGame::ObjectRender()
{
	for(auto& chara : _chara)
	{
		if(chara->IsAlive())
		{
			chara->Render();
		}
	}

	for(auto& enemy : _enemyBase)
	{
		if(enemy->IsAlive())
		{
			enemy->Render();
		}
	}


	// オブジェクトを描画
	for(auto& object : _object)
	{
		object->Render();
	}

	// プレイヤーの描画（フラグに応じて片方のみ）
	for(auto& player_base : _playerBase)
	{
		if(_bShowTanuki)
		{
			if(player_base.get() == _playerTanuki.get() && player_base->IsAlive())
			{
				player_base->Render();
			}
		}
		else if(_showMonoPlayer)
		{
			if(player_base.get() == _playerMono.get() && player_base->IsAlive())
			{
				player_base->Render();
			}
		}
		else
		{
			if(player_base.get() == _player.get() && player_base->IsAlive())
			{
				player_base->Render();
			}
		}
	}
	// キャラクターの影描画
	for(auto& shadow : _charaShadow)
	{
		if(shadow)
		{
			shadow->Render();
		}
	}

	// エフェクト
	for(auto& effectBase : _effectBase)
	{
		effectBase->Render();
	}

	// UIを描画
	for(auto& ui_base : _uiBase)
	{
		ui_base->Render();
	}

	// 索敵システムの描画
	if(_enemySensor)
	{
		_enemySensor->Render();
		_enemySensor->RenderDetectionUI();
	}

	for(auto& enemy : _enemyBase)
	{
		if(enemy->IsAlive())
		{
			// 音センサーの描画
			if(enemy->GetEnemySoundSensor())
			{
				enemy->GetEnemySoundSensor()->Render();
			}
		}
	}

	// 各敵のセンサーを個別に描画
	for(auto& enemy : _enemyBase)
	{
		if(enemy->IsAlive() && enemy->GetEnemySensor())
		{
			enemy->GetEnemySensor()->Render();
			enemy->GetEnemySensor()->RenderDetectionUI();
		}
	}

	return true;
}

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

bool ModeGame::CheckAllDetections()
{
	// 表示中のプレイヤーを選択（タヌキ / Mono / 通常）
	PlayerBase* player = nullptr;
	if(_bShowTanuki)
	{
		player = _playerTanuki.get();
	}
	else if(_showMonoPlayer)
	{
		player = _playerMono.get();
	}
	else
	{
		player = _player.get();
	}

	if(!player)
	{
		return false;
	}

	bool anyDetected = false;

	auto processContainer = [&](auto& container) -> bool
		{
			for(auto& item : container)
			{
				EnemyBase* eb = static_cast<EnemyBase*>(item.get());
				if(!eb || !eb->IsAlive())
				{
					continue;
				}

				auto sensor = eb->GetEnemySensor();
				if(!sensor)
				{
					continue;
				}

				// センサーに必要な情報をセット
				sensor->SetPos(eb->GetPos());
				sensor->SetDir(eb->GetDir());
				sensor->SetMap(_map.get());

				// センサー処理（追跡タイマー更新など）
				sensor->Process();

				// 音センサーも更新
				auto soundSensor = eb->GetEnemySoundSensor();
				if(soundSensor)
				{
					soundSensor->SetPos(eb->GetPos());
					soundSensor->Process();
				}

				// プレイヤーを使用して索敵判定を行う（表示中のプレイヤーが対象）
				bool detected = sensor->CheckPlayerDetection(player);

				if(detected)
				{
					anyDetected = true;
					eb->OnPlayerDetected(player->GetPos());
				}
				else
				{
					// センサーが追跡状態でなければ失見処理
					if(!sensor->IsChasing())
					{
						eb->OnPlayerLost();
					}
				}
			}
			return false;
		};

	processContainer(_enemyBase);

	_bTransCancel = anyDetected;

	return anyDetected;
}