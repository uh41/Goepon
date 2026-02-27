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
	// メインカメラ
	_originalCamera = _camera;
	// 演出カメラの初期化
	_cinematicCamera = std::make_unique<CinematicCamera>();
	_cinematicCamera->Initialize();
	

	//// マップ初期化
	//_map = std::make_shared<Map>();
	//_object.emplace_back(_map);

	//auto makimono = std::make_shared<Makimono>();
	//makimono->Initialize();          // モデル読み込み・当たり判定フレーム設定
	//makimono->SetCamera(_camera);
	//_makimono.emplace_back(makimono);

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

	_uiMakimono = std::make_shared<UiMakimono>();
	_uiMakimono->SetPlayer(_player.get());
	_uiBase.emplace_back(_uiMakimono);

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

	_doyaEffect->SetTargetPlayer(_playerTanuki.get());
	_nakiEffect->SetTargetPlayer(_playerTanuki.get());

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
		vec::Vec3 camDelta = vec3::VSub(_camera->GetPos(), _camera->GetTarget());

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
			_camera->SetTarget(target);
			_camera->SetPos(vec3::VAdd(target, camDelta));
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
		_transformAnimId = _playerTanuki->PlayAnimation("henge", false); 
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
			_doyaEffect->SetTargetPlayer(_player.get());
			_nakiEffect->SetTargetPlayer(_player.get());
			_player->Process(); // 変身直後の一フレーム更新

			// たぬ人間変身時の処理
			_changeTimeActive = true;// 時間制言を有効化
			_changeTimeLimit = 17.0f; // 変身時間をリセット
			_changeBlinkTimer = 0.0f; // 点滅タイマーリセット
			_changeBlinkVisible = true; // 点滅表示フラグリセット
			auto s = gGlobal._soundServer->Get("1");
			if(s && s->IsPlay())
			{
				s->Stop();
			}
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
			_doyaEffect->SetTargetPlayer(_playerMono.get());
			_nakiEffect->SetTargetPlayer(_playerMono.get());

			_playerMono->Process(); // 変身直後の一フレーム更新
			_hensinEffect->PlayEffect(_playerMono->GetPos());

			// たぬモノ変身時の処理
			_changeTimeActive = true;
			_changeTimeLimit = 12.0f;
			_changeBlinkTimer = 0.0f;
			_changeBlinkVisible = true;
			auto s = gGlobal._soundServer->Get("1");
			if(s && s->IsPlay())
			{
				s->Stop();
			}
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
			_playerTanuki->PlayAnimation("idle", true);
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

	// タヌキ表示時の人間への変身（入力または時間切れ）
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
			_playerTanuki->PlayAnimation("idle", true);
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
		// まきものを持ってないのに変身ボタンを押した場合のフィードバック（音のみ）
		auto soundNoMakimono = gGlobal._soundServer->Get("61");
		if(soundNoMakimono && !soundNoMakimono->IsPlay())
		{
			soundNoMakimono->Play();
		}
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
						/*if(_playerTanuki)
						{
							_playerTanuki->SubMakimono(1);
						}*/

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

	// 変身時間の点滅処理
	auto renderPlayerWithBlink = [this](PlayerBase* player)
		{
			if(player == nullptr || !player->IsAlive())
			{
				return;
			}

			const bool useBlink = (_changeTimeActive && _changeTimeLimit <= 10.0f);

			int modelHandle = player->GetModelHandle();
			if(modelHandle >= 0)
			{
				int materialNum = MV1GetMaterialNum(modelHandle);

				if(useBlink && !_changeBlinkVisible)
				{
					// 各マテリアルに赤色の加算ブレンドを設定
					for(int i = 0; i < materialNum; i++)
					{
						MV1SetMaterialDrawBlendMode(modelHandle, i, DX_BLENDMODE_ADD);
						MV1SetMaterialDrawBlendParam(modelHandle, i, 128);
						MV1SetMaterialDifColor(modelHandle, i, GetColorF(255, 0, 0, 255));
					}
				}
				else
				{
					// 通常のブレンドモードに戻す
					for(int i = 0; i < materialNum; i++)
					{
						MV1SetMaterialDrawBlendMode(modelHandle, i, DX_BLENDMODE_NOBLEND);
						MV1SetMaterialDrawBlendParam(modelHandle, i, 255);
						MV1SetMaterialDifColor(modelHandle, i, GetColorF(255, 255, 255, 255));
					}
				}
			}

			// 描画実行
			player->Render();
		};

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
			if(player_base.get() == _playerMono.get())
			{
				renderPlayerWithBlink(_playerMono.get());
			}
		}
		else
		{
			if(player_base.get() == _player.get())
			{
				renderPlayerWithBlink(_player.get());
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

	// UIが参照するプレイヤーを「現在表示中」に合わせる
	PlayerBase* currentPlayer = nullptr;
	if (_bShowTanuki) { currentPlayer = _playerTanuki.get(); }
	else if (_showMonoPlayer) { currentPlayer = _playerMono.get(); }
	else { currentPlayer = _player.get(); }

	if (_uiHp) { _uiHp->SetPlayer(currentPlayer); }
	if (_uiMakimono) { _uiMakimono->SetPlayer(currentPlayer); }

	// UIを描画
	for(auto& ui_base : _uiBase)
	{
		ui_base->Render();
	}

	// 各敵のセンサーを個別に描画
	for(auto& enemy : _enemyBase)
	{
		if(enemy->IsAlive() && enemy->GetEnemySensor())
		{
			enemy->GetEnemySensor()->Render();
		}
	}

	return true;
}

bool ModeGame::ChangeBGM()
{
	// 全ての敵の追跡状態をチェック
	bool isChase = false;
	for(auto& enemy : _enemyBase)
	{
		if(!enemy || !enemy->IsAlive())
		{
			continue;
		}

		// 各敵のセンサーが追跡中かチェック
		if(enemy->GetEnemySensor() && enemy->GetEnemySensor()->IsChasing())
		{
			isChase = true;
			break; // 1体でも追跡中ならBGM切り替え
		}
	}

	// BGMチェンジ処理
	if(!_isChengeBgm && isChase)
	{
		if(_bgmInitialize && _bgmInitialize->IsPlay())
		{
			_bgmInitialize->Stop();
		}
		if(_bgmChenge && !_bgmChenge->IsPlay())
		{
			_bgmChenge->Play();
		}
		_isChengeBgm = true;
	}
	else if(_isChengeBgm && !isChase)
	{
		if(_bgmChenge && _bgmChenge->IsPlay())
		{
			_bgmChenge->Stop();
		}
		if(_bgmInitialize && !_bgmInitialize->IsPlay())
		{
			_bgmInitialize->Play();
		}
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

	// Mono の「動いたか」を判定する閾値（ワールド単位）
	constexpr float kMonoMoveDetectThreshold = 0.1f;

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

				// 視覚検知判定
				bool detected = false;

				// --- PlayerMono 表示時の特別処理 ---
				if(_showMonoPlayer && dynamic_cast<PlayerMono*>(player))
				{
					// PlayerMono のときは「扇形内にいて、かつプレイヤーが動いた場合のみ」検知させる
					// プレイヤーのカプセル情報を取得
					vec::Vec3 playerPos = player->GetPos();
					vec::Vec3 capsuleTop = vec3::VAdd(playerPos, vec3::VGet(0.0f, player->GetColSubY(), 0.0f));
					vec::Vec3 capsuleBottom = vec3::VAdd(playerPos, vec3::VGet(0.0f, -player->GetColSubY(), 0.0f));
					float capsuleRadius = player->GetCollisionR();

					if (sensor->IsPlayerInDetectionRangeWithCapsule(playerPos, capsuleTop, capsuleBottom, capsuleRadius))
					{
						// プレイヤーの移動量をチェック（座標差だけでなく入力でも判定）
						vec::Vec3 delta = vec3::VSub(player->GetPos(), player->GetOldPos());
						float moved = vec3::VSize(delta);

						// 入力ベクトルがあるか（アナログ/十字キー）を判定
						bool inputMoving = (vec3::VSize(player->GetInputVector()) > 0.001f);

						// 座標差が閾値超過、もしくは入力があるなら「動いた」とみなす
						if(moved > kMonoMoveDetectThreshold || inputMoving)
						{
							detected = sensor->CheckPlayerDetection(player);
						}
						else
						{
							// 静止しているので検知しない（ただしセンサー追跡中は維持）
							detected = false;
						}
					}
					else
					{
						detected = false;
					}
				}
				else if(!isHumanForm)
				{
					// 非人状態：既存の通常判定をそのまま使用
					detected = sensor->CheckPlayerDetection(player);
				}
				else
				{
					if(player != nullptr && sensor != nullptr)
					{
						// 人状態：プレイヤーの尻尾(後方)を見られたときのみ検知する
						vec::Vec3 playerPos = player->GetPos();
						vec::Vec3 capsuleTop = vec3::VAdd(playerPos, vec3::VGet(0.0f, player->GetColSubY(), 0.0f));
						vec::Vec3 capsuleBottom = vec3::VAdd(playerPos, vec3::VGet(0.0f, -player->GetColSubY(), 0.0f));
						float capsuleRadius = player->GetCollisionR();

						if (sensor->IsPlayerInDetectionRangeWithCapsule(playerPos, capsuleTop, capsuleBottom, capsuleRadius))
						{
							vec::Vec3 toEnemy = vec3::VSub(eb->GetPos(), player->GetPos());
							toEnemy.y = 0.0f;
							if(vec3::VSize(toEnemy) > 0.0001f)
							{
								vec::Vec3 toEnemyNorm = vec3::VNorm(toEnemy);

								vec::Vec3 playerForward = player->GetDir();
								playerForward.y = 0.0f;
								if(vec3::VSize(playerForward) > 0.0001f)
								{
									playerForward = vec3::VNorm(playerForward);

									const float backDotThreshold = 0.0f;
									float dot = vec::Vec3::Dot(playerForward, toEnemyNorm);
									if(dot <= backDotThreshold)
									{
										detected = sensor->CheckPlayerDetection(player);
									}
								}
							}
						}
					}
					else
					{
						detected = false;
					}
				}

				// 検出結果に応じた処理
				if(detected)
				{
					anyDetected = true;
					if(player != nullptr) // NULLチェックを追加
					{
						eb->OnPlayerDetected(player->GetPos());
						_hatenaEffect->ResetEnemyEffect(eb);
						_nakiEffect->SetTargetPlayer(player);
						_nakiEffect->PlayEffect(player->GetPos());
					}

					// --- 追加: PlayerMono が検知されたら即時モノ->タヌキに切替 ---
					if(_showMonoPlayer && dynamic_cast<PlayerMono*>(player))
					{
						if(_playerTanuki && player != _playerTanuki.get())
						{
							_showMonoPlayer = false;
							_bShowTanuki = true;

							_playerTanuki->SetPos(player->GetPos());
							_playerTanuki->SetDir(player->GetDir());
							_playerTanuki->_status = CharaBase::STATUS::WAIT;
							_playerTanuki->PlayAnimation("goepon_idle", true);
							_playerTanuki->Process();

							_hensinEffect->PlayEffect(_playerTanuki->GetPos());
							_walkEffect->SetPlayerPos(_playerTanuki.get());
							_aseEffect->SetPlayer(_playerTanuki.get());

							// タイマー等リセット（モノ表示からの即時戻しは時間制限を扱わない）
							_changeTimeActive = false;
							_changeTimeLimit = 0.0f;
							_changeBlinkTimer = 0.0f;
							_changeBlinkVisible = true;

							auto soundFinish = gGlobal._soundServer->Get("3");
							if(soundFinish && !soundFinish->IsPlay())
							{
								soundFinish->Play();
							}
						}
					}

					// 人状態で尻尾（後方）を見られた場合、強制的にタヌキ表示へ切替
					if(isHumanForm)
					{
						if(_playerTanuki && player != _playerTanuki.get())
						{
							_showMonoPlayer = false;
							_bShowTanuki = true;

							_playerTanuki->SetPos(player->GetPos());
							_playerTanuki->SetDir(player->GetDir());
							_playerTanuki->_status = CharaBase::STATUS::WAIT;
							_playerTanuki->PlayAnimation("idle", true);
							_playerTanuki->Process();
							reEffect = true;

							if(reEffect)
							{
								_hensinEffect->PlayEffect(_playerTanuki->GetPos());
								_walkEffect->SetPlayerPos(_playerTanuki.get());
								_aseEffect->SetPlayer(_playerTanuki.get());
							}

							_changeTimeActive = false;
							_changeTimeLimit = 0.0f;
							_changeBlinkTimer = 0.0f;
							_changeBlinkVisible = true;

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