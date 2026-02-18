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
	// カメラ初期化
	_camera = new Camera();
	_camera->Initialize();

	//// マップ初期化
	//_map = std::make_shared<Map>();
	//_object.emplace_back(_map);

	auto makimono = std::make_shared<Makimono>();
	makimono->Initialize();          // モデル読み込み・当たり判定フレーム設定
	makimono->SetCamera(_camera);
	_makimono.emplace_back(makimono);

	// プレイヤー初期化
	_player = std::make_shared<Player>();
	_playerBase.emplace_back(_player);
	_playerTanuki = std::make_shared<PlayerTanuki>();
	_playerBase.emplace_back(_playerTanuki);
	_playerMono = std::make_shared<PlayerMono>();
	_playerBase.emplace_back(_playerMono);

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
	_hensinEffect = std::make_shared<HensinEffect>();
	_effectBase.emplace_back(_hensinEffect);
	_walkEffect = std::make_shared<WalkEffect>();
	_effectBase.emplace_back(_walkEffect);
	_findEffect = std::make_shared<FindEffect>();
	_effectBase.emplace_back(_findEffect);
	_hatenaEffect = std::make_shared<HatenaEffect>();
	_effectBase.emplace_back(_hatenaEffect);
	_aseEffect = std::make_shared<AseEffect>();
	_effectBase.emplace_back(_aseEffect);
	_doyaEffect = std::make_shared<DoyaEffect>();
	_effectBase.emplace_back(_doyaEffect);
	_nakiEffect = std::make_shared<NakiEffect>();
	_effectBase.emplace_back(_nakiEffect);

	_sound3D = std::make_shared<SoundServer3D>(gGlobal._soundServer);
	_sound3D->SetRadius(768.0f);

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

	// 点滅間隔の初期化（秒） — 明示的に初期化しておく
	_changeBlinkInterval = 0.1f; // 0.5秒ごとに点滅
	_changeBlinkTimer = 0.0f;
	_changeBlinkVisible = true;

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
		shadow->SetTargetChara([eb]() -> CharaBase* { return StCas<CharaBase*>(eb.get()); });
		_charaShadow.emplace_back(shadow);
	}

	return true;
}

// プレイヤー変身処理
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
	auto soundHenshin = [this]()
		{
			auto henshinSound = gGlobal._soundServer->Get("2");
			if(henshinSound && !henshinSound->IsPlay())
			{
				henshinSound->Play();
			}
		};
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
		soundHenshin();
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
			_walkEffect->SetPlayerPos(_player.get());
			_aseEffect->SetPlayer(_player.get());
			// タヌキから人間への変身完了時に音波を発生
			for(auto& enemy : _enemyBase)
			{
				if(enemy->IsAlive())
				{
					// 変身中は敵の音検知を無効化（音波を発生させない）
					if(enemy->GetEnemySoundSensor())
					{
						enemy->GetEnemySoundSensor()->SetSoundLevel(0);
					}
					_hatenaEffect->PlayOnce(enemy.get());
				}
			}
			_player->Process(); // 変身直後の一フレーム更新

			// たぬ人間変身時の処理
			_changeTimeActive = true;// 時間制言を有効化
			_changeTimeLimit = 20.0f; // 変身時間をリセット
			_changeBlinkTimer = 0.0f; // 点滅タイマーリセット
			_changeBlinkVisible = true; // 点滅表示フラグリセット
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
			_hensinEffect->PlayEffect(_playerMono->GetPos());
			_walkEffect->SetPlayerPos(_playerMono.get());
			_aseEffect->SetPlayer(_playerMono.get());

			// タヌキから人間への変身完了時に音波を発生
			for(auto& enemy : _enemyBase)
			{
				if(enemy->IsAlive())
				{
					// 変身中は敵の音検知を無効化（音波を発生させない）
					if(enemy->GetEnemySoundSensor())
					{
						enemy->GetEnemySoundSensor()->SetSoundLevel(0);
					}
					_hatenaEffect->PlayOnce(enemy.get());
				}
			}
			_playerMono->Process(); // 変身直後の一フレーム更新
			_hensinEffect->PlayEffect(_playerMono->GetPos());

			// たぬモノ変身時の処理
			_changeTimeActive = true;
			_changeTimeLimit = 10.0f;
			_changeBlinkTimer = 0.0f;
			_changeBlinkVisible = true;
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

	// モノ表示時の自動切替（入力または時間切れ）
	if(_showMonoPlayer)
	{
		// PAD_INPUT_3 が押された、または変身タイマーが有効で時間切れならタヌキへ切替
		if((trg & PAD_INPUT_3) || (_changeTimeActive && _changeTimeLimit <= 0.0f))
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
			_walkEffect->SetPlayerPos(_playerTanuki.get());
			_aseEffect->SetPlayer(_playerTanuki.get());
			auto soundHenshin = gGlobal._soundServer->Get("2");
			if(soundHenshin && !soundHenshin->IsPlay())
			{
				soundHenshin->Play();
			}

			// タイマーが動いてたらリセット
			_changeTimeActive = false; // 時間制限を無効化
			_changeTimeLimit = 0.0f;
			_changeBlinkTimer = 0.0f;
			_changeBlinkVisible = true;
			return true;
		}
	}
	if(!_bShowTanuki && !_showMonoPlayer)
	{
		if((trg & PAD_INPUT_4) || (_changeTimeActive && _changeTimeLimit <= 0.0f))
		{
			// 人間 -> タヌキ（再度ボタンで戻す）
			_bShowTanuki = true;
			_showMonoPlayer = false;

			_playerTanuki->SetPos(_player->GetPos());
			_playerTanuki->SetDir(_player->GetDir());
			_playerTanuki->_status = CharaBase::STATUS::WAIT;
			_playerTanuki->PlayAnimation("goepon_idle", true);
			_playerTanuki->Process();

			_hensinEffect->PlayEffect(_playerTanuki->GetPos());
			_walkEffect->SetPlayerPos(_playerTanuki.get());
			_aseEffect->SetPlayer(_playerTanuki.get());

			auto soundFinish = gGlobal._soundServer->Get("3");
			if(soundFinish && !soundFinish->IsPlay())
			{
				soundFinish->Play();
			}

			// タイマーが動いてたらリセット（人状態の点滅等を止める）
			_changeTimeActive = false;
			_changeTimeLimit = 0.0f;
			_changeBlinkTimer = 0.0f;
			_changeBlinkVisible = true;

			return true;
		}
	}
	// 巻物取得チェック
	PlayerBase* currentPlayer = nullptr;
	if(_bShowTanuki) { currentPlayer = _playerTanuki.get(); }
	else if(_showMonoPlayer) { currentPlayer = _playerMono.get(); }
	else { currentPlayer = _player.get(); }

	const bool hasMakimono = (currentPlayer != nullptr && currentPlayer->GetMakimonoCount() > 0);
	const bool pushBottan  = (trg & PAD_INPUT_3) || (trg & PAD_INPUT_4); // 変身ボタンが押されているか

	if(!hasMakimono && pushBottan)
	{
		// もしSEやUIを表示する場合はここに追加
	}
	else
	{
		// PAD_INPUT_3: タヌキ <-> モノ 切替
		if(trg & PAD_INPUT_3)
		{
			// 無効化：現在「モノ」表示中なら PAD_INPUT_3 を無効化する
			if(!_bShowTanuki && !_showMonoPlayer)
			{
				// プレイヤー（人間）時は無効化、何もしない
			}
			else
			{
				// タヌキ表示中ならモノに切り替え（変身開始）
				if(_bShowTanuki)
				{
					// まきものを1つ消費する
					if(_playerTanuki)
					{
						_playerTanuki->SubMakimono(1);
					}
					// タヌキ -> モノ へ変身開始
					if(PlayerTransformToTanuki(false))
					{
						return true;
					}
				}
				//else if(_showMonoPlayer)
				//{
				//	// (既存) モノ -> タヌキ は即時切替（アニメなし）
				//	_showMonoPlayer = false;
				//	_bShowTanuki = true;
				//	_playerTanuki->SetPos(_playerMono->GetPos());
				//	_playerTanuki->SetDir(_playerMono->GetDir());
				//	_playerTanuki->_status = CharaBase::STATUS::WAIT;
				//	_playerTanuki->PlayAnimation("goepon_idle", true);
				//	_playerTanuki->Process();
				//	_hensinEffect->PlayEffect(_playerTanuki->GetPos());
				//	_walkEffect->SetPlayerPos(_playerTanuki.get());
				//	_aseEffect->SetPlayer(_playerTanuki.get());
				//	auto soundFinish = gGlobal._soundServer->Get("3");
				//	if(soundFinish && !soundFinish->IsPlay())
				//	{
				//		soundFinish->Play();
				//	}

				//	// タイマーが動いてたらリセット
				//	_changeTimeActive = false;// 時間制限を無効化
				//	_changeTimeLimit = 0.0f;
				//	_changeBlinkTimer = 0.0f;
				//	_changeBlinkVisible = true;
				//	return true;
				//}
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
						// まきものを1つ消費する
						if(_playerTanuki)
						{
							_playerTanuki->SubMakimono(1);
						}

						if(PlayerTransformToTanuki(true))
						{
							return true;
						}
					}
					//else
					//{
					//	// 人間 -> タヌキ（再度ボタンで戻す）
					//	_bShowTanuki = true;
					//	_showMonoPlayer = false;

					//	_playerTanuki->SetPos(_player->GetPos());
					//	_playerTanuki->SetDir(_player->GetDir());
					//	_playerTanuki->_status = CharaBase::STATUS::WAIT;
					//	_playerTanuki->PlayAnimation("goepon_idle", true);
					//	_playerTanuki->Process();

					//	_hensinEffect->PlayEffect(_playerTanuki->GetPos());
					//	_walkEffect->SetPlayerPos(_playerTanuki.get());
					//	_aseEffect->SetPlayer(_playerTanuki.get());

					//	auto soundFinish = gGlobal._soundServer->Get("3");
					//	if(soundFinish && !soundFinish->IsPlay())
					//	{
					//		soundFinish->Play();
					//	}

					//	// タイマーが動いてたらリセット（人状態の点滅等を止める）
					//	_changeTimeActive = false;
					//	_changeTimeLimit = 0.0f;
					//	_changeBlinkTimer = 0.0f;
					//	_changeBlinkVisible = true;

					//	return true;
					//	
					//}
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

	// 宝箱処理
	for (auto& t : _treasure)
	{
		if (t) t->Process();
	}

	// 巻物処理
	for(auto& makimono : _makimono)
	{
		makimono->Process();
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
bool ModeGame::ObjectRender()
{
	for(auto& chara : _chara)
	{
		if(chara->IsAlive())
		{
			chara->Render();
		}
	}

	// 敵の描画
	for(auto& enemy : _enemyBase)
	{
		if(enemy->IsAlive())
		{
			enemy->Render();
		}
	}

	// 宝箱の描画
	for (auto& t : _treasure)
	{
		if (t) t->Render();
	}

	// 巻物の描画
	for(auto& makimono : _makimono)
	{
		if(makimono) makimono->Render();
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
				if(_changeTimeActive && _changeTimeLimit <= 10.0f)
				{
					// 点滅中で「非表示側」のタイミングなら描画しない
					if(!_changeBlinkVisible)
					{

						continue;
					}
				}
				player_base->Render();
			}
		}
		else
		{
			// 人間プレイヤー描画時に、タイマー点滅中なら描画をスキップする判定を入れる
			if(player_base.get() == _player.get() && player_base->IsAlive())
			{
				if(_changeTimeActive && _changeTimeLimit <= 10.0f)
				{
					// 点滅中で「非表示側」のタイミングなら描画しない
					if(!_changeBlinkVisible)
					{

						continue;
					}
				}
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

	// 人状態かどうかを判定
	bool isHumanForm = (!_bShowTanuki && !_showMonoPlayer);

	bool anyDetected = false;	// いずれかの敵が検知したかどうか
	bool reEffect;				// エフェクト再設定フラグ

	auto processContainer = [&](auto& container) -> bool
		{
			for(auto& item : container)
			{
				EnemyBase* eb = StCas<EnemyBase*>(item.get());
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
				sensor->SetMap(_objectServer->GetMap());

				// センサー処理（追跡タイマー更新など）
				sensor->Process();

				// 音センサーも更新
				auto soundSensor = eb->GetEnemySoundSensor();
				if(soundSensor)
				{
					soundSensor->SetPos(eb->GetPos());
					soundSensor->Process();
				}

				// 視覚検知判定
				bool detected = false;

				if (!isHumanForm)
				{
					// 非人状態：既存の通常判定をそのまま使用
					detected = sensor->CheckPlayerDetection(player);
				}
				else
				{
					// 人状態：プレイヤーの尻尾(後方)を見られたときのみ検知する
					// 敵から見てプレイヤーが索敵範囲内か
					if (sensor->IsPlayerInDetectionRange(player->GetPos()))
					{
						// 敵がプレイヤーの「後方」にいるかチェック
						vec::Vec3 toEnemy = vec3::VSub(eb->GetPos(), player->GetPos());
						toEnemy.y = 0.0f;
						if (vec3::VSize(toEnemy) > 0.0001f)
						{
							vec::Vec3 toEnemyNorm = vec3::VNorm(toEnemy);

							vec::Vec3 playerForward = player->GetDir();
							playerForward.y = 0.0f;
							if (vec3::VSize(playerForward) > 0.0001f)
							{
								playerForward = vec3::VNorm(playerForward);

								// 内積によって後方かどうかを判定
								// playerForward と toEnemyNorm が一直線で逆向きなら内積 = -1
								// threshold を 0 にすると、正面90度以外が後方扱いになる
								const float backDotThreshold = 0.0f; //負の値が大きいほど範囲が狭くなる
								float dot = vec::Vec3::Dot(playerForward, toEnemyNorm);
								if (dot <= backDotThreshold)
								{
									// 実際の検知処理（副作用：検出情報の更新など）
									detected = sensor->CheckPlayerDetection(player);
								}
							}
						}
					}
				}

				// 検出結果に応じた処理
				if(detected)
				{
					anyDetected = true;
					eb->OnPlayerDetected(player->GetPos());
					_hatenaEffect->ResetEnemyEffect(eb);
					_nakiEffect->PlayEffect(player->GetPos());

					// 人状態で尻尾（後方）を見られた場合、強制的にタヌキ表示へ切替
					if (isHumanForm)
					{
						// _playerTanuki が存在し、既にタヌキ表示でなければ切替
						if (_playerTanuki && player != _playerTanuki.get())
						{
							_showMonoPlayer = false;
							_bShowTanuki = true;

							// 位置・向きを引き継ぐ
							_playerTanuki->SetPos(player->GetPos());
							_playerTanuki->SetDir(player->GetDir());
							_playerTanuki->_status = CharaBase::STATUS::WAIT;
							_playerTanuki->PlayAnimation("goepon_idle", true);
							_playerTanuki->Process();
							reEffect = true;

							// 変身エフェクト等を再設定
							if (reEffect)
							{
								_hensinEffect->PlayEffect(_playerTanuki->GetPos());
								_walkEffect->SetPlayerPos(_playerTanuki.get());
								_aseEffect->SetPlayer(_playerTanuki.get());
							}
							auto soundFinish = gGlobal._soundServer->Get("3");
							if(soundFinish && !soundFinish->IsPlay())
							{
								soundFinish->Play();
							}
						}
					}
				}
				else
				{
					// センサーが追跡状態でなければ失見処理
					if(!sensor->IsChasing())
					{
						if(eb->IsDetectPlayer())
						{
							_hatenaEffect->PlayOnce(eb);
						}
						eb->OnPlayerLost();
					}
				}
			}
			return false;
		};

	processContainer(_enemyBase);

	if(!anyDetected && _nakiEffect)
	{
		_nakiEffect->ResetEffect();
		reEffect = false;
	}

	_bTransCancel = anyDetected;

	return anyDetected;
}