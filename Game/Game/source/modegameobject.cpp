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

void ModeGame::RequestTransformToMono()
{
	if(_bTransCancel)
	{
		return;
	}
	// 要求フラグを立てるだけ。実際の消費/変身は PlayerTransform() 内で行う
	_requestedTransformToMono = true;
}

void ModeGame::RequestTransformToHuman()
{
	if(_bTransCancel)
	{
		return;
	}

	_requestedTransformToHuman = true;
}

void ModeGame::RequestReturnToTanukiFromHuman()
{
	_requestedReturnToTanuki = true;
}

bool ModeGame::IsTransforming() const
{
	return _isTransformToHuman || _isTransformToMono ||  (_transformAnimId != -1);
}

bool ModeGame::IsTransformRequested() const
{
	return _requestedTransformToMono || _requestedTransformToHuman || _requestedReturnToTanuki;
}

void ModeGame::CancelRequestedTransform()
{
	if(IsTransforming())
	{
		return;
	}

	_requestedTransformToMono = false;
	_requestedTransformToHuman = false;
	_requestedReturnToTanuki = false;
}

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
	_savedCamera = _camera;
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
	_henshinUi = std::make_shared<HenshinUi>();
	_henshinUi->SetOwner(this);
	_uiBase.emplace_back(_henshinUi);
	_uiMakimono = std::make_shared<UiMakimono>();
	_uiMakimono->SetPlayer(_player.get());
	_uiBase.emplace_back(_uiMakimono);
	_counterUi = std::make_shared<CounterUi>();
	_uiBase.emplace_back(_counterUi);
	_attackUi = std::make_shared<AttackUi>();
	_attackUi->Show(_player.get()->GetPos());
	_uiBase.emplace_back(_attackUi);
	_treasureOpenUi = std::make_shared<TreasureOpenUi>();
	_uiBase.emplace_back(_treasureOpenUi);
	_dashUi = std::make_shared<DashUi>();
	_dashUi->SetPlayer(_playerTanuki.get());
	_uiBase.emplace_back(_dashUi);
	_treasureUi = std::make_shared<TreasureUi>();
	_treasureUi->SetTreasureList(_treasureBase);
	_uiBase.emplace_back(_treasureUi);

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
	_doyaEffect = std::make_shared<DoyaEffect>();
	_effectBase.emplace_back(_doyaEffect);
	_TreasureOpenEffect = std::make_shared<TreasureopenEffect>();
	_effectBase.emplace_back(_TreasureOpenEffect);
	_nakiEffect = std::make_shared<NakiEffect>();
	_effectBase.emplace_back(_nakiEffect);
	_shirimochiEffect = std::make_shared<ShirimochiEffect>();
	_effectBase.emplace_back(_shirimochiEffect);
	_stunEffect = std::make_shared<StunEffect>();
	_effectBase.emplace_back(_stunEffect);
	_savePointEffect = std::make_shared<SavePointEffect>();
	_effectBase.emplace_back(_savePointEffect);
	_makimonoGetEffect = std::make_shared<MakimonoGetEffect>();
	_effectBase.emplace_back(_makimonoGetEffect);
	_goalEffect = std::make_shared<GoalEffect>();
	_goalEffect->SetGoal(_goal);
	_effectBase.emplace_back(_goalEffect);

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

	if(_goal)
	{
		_goal->SetVisible(true);           // 描画はする
		_goal->SetCollisionEnabled(false); // 当たり判定は無効（宝を全部取ったら有効化する）
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

	// 点滅間隔の初期化（秒） — 明示的に初期化しておく
	_changeBlinkInterval = 0.1f; // 0.5秒ごとに点滅
	_changeBlinkTimer = 0.0f;
	_changeBlinkVisible = true;

	_requestedTransformToMono = false;
	_requestedTransformToHuman = false;
	_requestedReturnToTanuki = false;
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
			_player->SetRotationY(atan2f(-_playerTanuki->GetDir().x, -_playerTanuki->GetDir().z));
			_player->SetMakimonoCount(_playerTanuki->GetMakimonoCount());
			_hensinEffect->PlayEffect(_player->GetPos());
			_walkEffect->SetPlayerPos(_player.get());
			_doyaEffect->SetTargetPlayer(_player.get());
			_nakiEffect->SetTargetPlayer(_player.get());
			//_aseEffect->StopPlaying();
			//_aseEffect->SetPlayer(nullptr);
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
			_playerMono->SetRotationY(atan2f(-_playerTanuki->GetDir().x, -_playerTanuki->GetDir().z));
			_playerMono->SetMakimonoCount(_playerTanuki->GetMakimonoCount());
			_hensinEffect->PlayEffect(_playerMono->GetPos());
			_walkEffect->SetPlayerPos(_playerMono.get());
			_doyaEffect->SetTargetPlayer(_playerMono.get());
			_nakiEffect->SetTargetPlayer(_playerMono.get());
			//_aseEffect->StopPlaying();
			//_aseEffect->SetPlayer(nullptr);

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

bool ModeGame::RequestTransform(HenshinUi::Select select)
{
	// UI からの要求はタヌキ表示中にのみ有効（表示がタヌキでない場合は無視）
	if(!_bShowTanuki || !_playerTanuki)
	{
		return false;
	}

	PlayerTanuki* tanuki = _playerTanuki.get();
	if(!tanuki)
	{
		return false;
	}

	switch(select)
	{
	case HenshinUi::Select::TANUMONO:
	{
		// モノへ変身する場合はタヌキの巻物を消費する
		if(tanuki->GetMakimonoCount() > 0)
		{
			tanuki->SubMakimono(1); // 巻物を消費して変身開始
			return PlayerTransformToTanuki(false); // false = モノへ
		}
		else
		{
			// 巻物がない場合は効果音のみ（既存挙動）
			auto soundNoMakimono = gGlobal._soundServer->Get("61");
			if(soundNoMakimono && !soundNoMakimono->IsPlay())
			{
				soundNoMakimono->Play();
			}
			return false;
		}
	}
	case HenshinUi::Select::TANUBITO:
	{
		// タヌキ表示中のみ人間へ変身
		return PlayerTransformToTanuki(true); // true = 人間へ
	}
	default:
		return false;
	}
}

bool ModeGame::PlayerTransform()
{
	// AnimationManager の更新は呼び出し側で行われている前提（modegame.cpp）
	int trg = ApplicationMain::GetInstance()->GetTrg();

	// 変身進行中は入力に関係なく毎フレーム進行させる
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

	// モノ表示時の自動切替（入力または時間切れ）
	if(_showMonoPlayer)
	{
		// PAD入力による切替は廃止。UI からの要求で行うようにする。
		if((_changeTimeActive && _changeTimeLimit <= 0.0f))
		{
			// モノ -> タヌキ は即時切替（アニメなし）
			_showMonoPlayer = false;
			_bShowTanuki = true;
			_playerTanuki->SetPos(_playerMono->GetPos());
			_playerTanuki->SetDir(_playerMono->GetDir());
			_playerTanuki->_status = CharaBase::STATUS::WAIT;
			_playerTanuki->SetMakimonoCount(_playerMono->GetMakimonoCount());
			_playerTanuki->SetRotationY(atan2f(-_playerMono->GetDir().x, -_playerMono->GetDir().z));
			_playerTanuki->PlayAnimation("idle", true);
			_playerTanuki->Process();
			_hensinEffect->PlayEffect(_playerTanuki->GetPos());
			_walkEffect->SetPlayerPos(_playerTanuki.get());
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

	// human 表示時 -> tanuki 即時戻し（UI 経由）
	if(_requestedReturnToTanuki)
	{
		_requestedReturnToTanuki = false;
		// タヌキでない表示（人間 or モノ）の場合、どちらからでも即時タヌキへ戻す
		if(!_bShowTanuki)
		{
			// 元の表示状態を保持しておく（モノか人か）
			bool wasMono = _showMonoPlayer;
			bool wasHuman = (!_bShowTanuki && !_showMonoPlayer);

			// タヌキ表示へ切替（モノ表示は解除）
			_bShowTanuki = true;
			_showMonoPlayer = false;

			// 位置・向きを元の表示からタヌキに引き継ぐ
			PlayerBase* srcPlayer = nullptr;
			if(wasMono && _playerMono) srcPlayer = _playerMono.get();
			else if(wasHuman && _player) srcPlayer = _player.get();

			if(srcPlayer && _playerTanuki)
			{
				_playerTanuki->SetPos(srcPlayer->GetPos());
				_playerTanuki->SetDir(srcPlayer->GetDir());
				_playerTanuki->SetRotationY(atan2f(-srcPlayer->GetDir().x, -srcPlayer->GetDir().z));
				_playerTanuki->_status = CharaBase::STATUS::WAIT;

				_playerTanuki->SetMakimonoCount(srcPlayer->GetMakimonoCount());

				// モノから戻る場合もタヌキの待機アニメーションを再生する
				_playerTanuki->PlayAnimation("idle", true);
				_playerTanuki->Process();
			}

			_hensinEffect->PlayEffect(_playerTanuki->GetPos());
			_walkEffect->SetPlayerPos(_playerTanuki.get());
			//_aseEffect->SetPlayer(_playerTanuki.get());

			auto soundFinish = gGlobal._soundServer->Get("3");
			if(soundFinish && !soundFinish->IsPlay())
			{
				soundFinish->Play();
			}

			_changeTimeActive = false;
			_changeTimeLimit = 0.0f;
			_changeBlinkTimer = 0.0f;
			_changeBlinkVisible = true;

			return true;
		}
	}

	// UI 経由の変身要求処理（パッド入力分岐は廃止）
	if(_requestedTransformToMono)
	{
		_requestedTransformToMono = false;
		// タヌキ表示中のみ許可
		if(_bShowTanuki)
		{
			if(_playerTanuki && _playerTanuki->GetMakimonoCount() > 0)
			{
				// 巻物消費して変身開始
				_playerTanuki->SubMakimono(1);
				if(PlayerTransformToTanuki(false))
				{
					return true;
				}
			}
			else
			{
				auto soundNoMakimono = gGlobal._soundServer->Get("61");
				if(soundNoMakimono && !soundNoMakimono->IsPlay())
				{
					soundNoMakimono->Play();
				}
			}
		}
	}

	if(_requestedTransformToHuman)
	{
		_requestedTransformToHuman = false;
		// タヌキ表示中のみ開始
		if(_bShowTanuki)
		{
			// 巻物がある時だけ人間へ変身（巻物消費）
			if(_playerTanuki && _playerTanuki->GetMakimonoCount() > 0)
			{
				_playerTanuki->SubMakimono(1);
				if(PlayerTransformToTanuki(true))
				{
					return true;
				}
			}
			else
			{
				// 巻物がない場合は効果音のみ
				auto soundNoMakimono = gGlobal._soundServer->Get("61");
				if(soundNoMakimono && !soundNoMakimono->IsPlay())
				{
					soundNoMakimono->Play();
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
			// プレイヤーからの距離が遠すぎる敵は思考・移動・アニメーション更新を止める
			PlayerBase* player = nullptr;
			if (_bShowTanuki)
			{
				player = _playerTanuki.get();
			}
			else if (_showMonoPlayer)
			{
				player = _playerMono.get();
			}
			else
			{
				player = _player.get();
			}
			if (player)
			{
				vec::Vec3 vecToEnemy = vec3::VSub(enemy->GetPos(), player->GetPos());
				if (vec3::VSize(vecToEnemy) < 1600.0f) 
				{ // 復帰が不自然にならないように描画より少し広め
					enemy->Process();
				}
			}
			else 
			{
				enemy->Process();
			}
		}
	}

	// 宝箱処理
	for (auto& t : _treasureBase)
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

bool ModeGame::ObjectRender()
{
	PlayerBase* currentPlayer = nullptr;

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
	for(auto& t : _treasureBase)
	{
		if(t) t->Render();
	}

	// 巻物の描画
	for(auto& makimono : _makimono)
	{
		if(makimono) makimono->Render();
	}

	// 宝箱の描画
	for(auto& t : _treasure)
	{
		if(t) t->Render();
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

	// UIが参照するプレイヤーを「現在表示中」に合わせる
	if(_bShowTanuki)
	{
		currentPlayer = _playerTanuki.get();
	}
	else if(_showMonoPlayer)
	{
		currentPlayer = _playerMono.get();
	}
	else
	{
		currentPlayer = _player.get();
	}

	if(_uiHp)
	{
		_uiHp->SetPlayer(currentPlayer);
	}
	if(_uiMakimono)
	{
		_uiMakimono->SetPlayer(currentPlayer);
	}
	if(_dashUi)
	{
		if(_bShowTanuki)
		{
			_dashUi->SetPlayer(_playerTanuki.get());
		}
		else
		{
			_dashUi->SetPlayer(nullptr);
		}
	}

	// ---- ここから先は「演出/ゲージ/UI」なので必ず影なしにする ----
	//SetUseShadowMap(0, -1);
	SetUseLighting(FALSE);

	// Effekseer（3D/2D）を影なしで描画（ワールドの後、UIの前）
	EffekseerManager::GetInstance()->Render();

	// 各敵のセンサーを個別に描画
	// プレイヤーから半径内にいる敵だけ索敵範囲を描画
	if(currentPlayer)
	{
		const float detectionRadius = 1000.0f;
		for(auto& enemy : _enemyBase)
		{
			if(!enemy || !enemy->IsAlive())
			{
				continue;
			}

			auto sensor = enemy->GetEnemySensor();
			if(!sensor)
			{
				continue;
			}

			// XZ平面で距離を測ってプレイヤーの半径内か判定
			vec::Vec3 vecToPlayer = vec3::VSub(enemy->GetPos(), currentPlayer->GetPos());
			vecToPlayer.y = 0.0f;
			const float dist = vec3::VSize(vecToPlayer);

			if(dist <= detectionRadius)
			{
				sensor->Render();
			}
		}
	}
	else
	{
		// プレイヤー不在なら従来どおり全部描画（安全策）
		for(auto& enemy : _enemyBase)
		{
			if(enemy->IsAlive() && enemy->GetEnemySensor())
			{
				enemy->GetEnemySensor()->Render();
			}
		}
	}

	if(_player)
	{
		// 長押し宝箱のゲージ描画
		for(const auto& treasureBase : _treasureBase)
		{
			if(treasureBase && treasureBase->IsVisible() && !treasureBase->IsOpen())
			{
				float treasureProgress = 0.0f;
				auto it = _treasureProgressMap.find(treasureBase.get());
				if(it != _treasureProgressMap.end())
				{
					treasureProgress = it->second;
				}

				treasureBase->RenderGauge(_player->GetPos(), treasureProgress);
			}
		}

		// 連打宝箱のゲージ描画
		for(const auto& treasureRapidFire : _treasureRapidFire)
		{
			if(treasureRapidFire && treasureRapidFire->IsVisible() && !treasureRapidFire->IsOpen())
			{
				treasureRapidFire->RenderGaugeRF(_player->GetPos(), 0.0f);
			}
		}
	}

	// エフェクト（各 EffectBase の Render は現状“描画自体”ではない想定）
	for(auto& effectBase : _effectBase)
	{
		effectBase->Render();
	}

	SetUseZBuffer3D(FALSE);
	SetWriteZBuffer3D(FALSE);

	// UIを描画（巻物UIを最初に、その後に残りのUIを描画）
	if(_treasureUi)
	{
		_treasureUi->GetHandleMakimono();
	}

	for(auto& ui_base : _uiBase)
	{
		ui_base->Render();
	}

	// 状態復帰（次の描画に影響を残さない）
	SetUseLighting(TRUE);
	//SetUseShadowMap(0, -1);

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
		if(_bgmChenge)
		{
			// 音量を上げてから再生（既にハンドルがあるならロードは発生しない）
			_bgmChenge->SetVolume(250);
			if(!_bgmChenge->IsPlay())
			{
				_bgmChenge->Play();
			}
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

bool ModeGame::ProcessEnemyContainer(at::vspc<EnemyBase>& container, PlayerBase* player, bool isHumanForm, bool& anyDetected, bool& reEffect)
{
	if(container.empty() || !player) return false;

	// Mono の「動いたか」を判定する閾値（ワールド単位）
	constexpr float kMonoMoveDetectThreshold = 0.1f;

	// 分散処理用の開始インデックスを保持（関数内で static にする）
	static size_t s_nextProcessIndex = 0;

	int nonChasingProcessedCount = 0;
	const int kMaxNormalChecksPerFrame = 1; // 1フレームに視覚判定する未発見状態の敵の数

	for(size_t i = 0; i < container.size(); ++i)
	{
		// ラウンドロビン方式で順番にアクセスする
		size_t currentIndex = (s_nextProcessIndex + i) % container.size();
		auto& item = container[currentIndex];

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

		// センサーに必要な情報を確実にセット（全員同期）
		sensor->SetPos(eb->GetPos());
		sensor->SetDir(eb->GetDir());
		sensor->SetMap(_objectServer->GetMap());

		// 現在の追跡状態を記憶しておく（これからの処理で更新される可能性があるため）
		const bool wasChasing = sensor->IsChasing();

		// 追跡中ではない敵の場合の時分割・距離スキップ判定
		if(!wasChasing)
		{
			// --- 負荷軽減のための距離判定 ---
			const float activeRadius = 1000.0f;
			vec::Vec3 vecToEnemy = vec3::VSub(eb->GetPos(), player->GetPos());
			vecToEnemy.y = 0.0f;
			if(vec3::VSize(vecToEnemy) > activeRadius)
			{
				continue; // 遠すぎる場合は検知処理をスキップ
			}

			// 既にこのフレームで判定人数の上限に達している場合はスキップ
			if(nonChasingProcessedCount >= kMaxNormalChecksPerFrame)
			{
				continue;
			}

			// 回数を消費し、次回のフレームで「この敵の次」から判定を始めるように記憶する
			nonChasingProcessedCount++;
			s_nextProcessIndex = (currentIndex + 1) % container.size();
		}

		// センサー処理（追跡タイマー更新など）
		sensor->Process();

		// 視覚検知判定
		bool detected = false;
		bool chaseStarted = false;

		// --- 犬の場合は人間形態でも全方向から検知可能 ---
		bool isEnemyDog = (dynamic_cast<EnemyDog*>(eb) != nullptr);

		// --- PlayerMono 表示時の特別処理 ---
		if(_showMonoPlayer && dynamic_cast<PlayerMono*>(player))
		{
			// PlayerMono のときは「扇形内にいて、かつプレイヤーが動いた場合のみ」検知させる
			vec::Vec3 playerPos = player->GetPos();
			vec::Vec3 capsuleTop = vec3::VAdd(playerPos, vec3::VGet(0.0f, player->GetColSubY(), 0.0f));
			vec::Vec3 capsuleBottom = vec3::VAdd(playerPos, vec3::VGet(0.0f, -player->GetColSubY(), 0.0f));
			float capsuleRadius = player->GetCollisionR();

			if(sensor->IsPlayerInDetectionRangeWithCapsule(playerPos, capsuleTop, capsuleBottom, capsuleRadius))
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
		else if(!isHumanForm || isEnemyDog)
		{
			// 非人状態 または 犬の場合：既存の通常判定をそのまま使用
			detected = sensor->CheckPlayerDetection(player);
		}
		else
		{
			if(player != nullptr && sensor != nullptr)
			{
				// 人状態（犬以外）：プレイヤーの尻尾(後方)を見られたときのみ検知する
				vec::Vec3 playerPos = player->GetPos();
				vec::Vec3 capsuleTop = vec3::VAdd(playerPos, vec3::VGet(0.0f, player->GetColSubY(), 0.0f));
				vec::Vec3 capsuleBottom = vec3::VAdd(playerPos, vec3::VGet(0.0f, -player->GetColSubY(), 0.0f));
				float capsuleRadius = player->GetCollisionR();

				if(sensor->IsPlayerInDetectionRangeWithCapsule(playerPos, capsuleTop, capsuleBottom, capsuleRadius))
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
			if(player != nullptr)
			{
				eb->OnPlayerDetected(player->GetPos());
				_hatenaEffect->ResetEnemyEffect(eb);

				const bool isChasingNow = sensor->IsChasing();
				chaseStarted = (!wasChasing && isChasingNow);

				if(eb->GetEnemySensor() && eb->GetEnemySensor()->IsChasing())
				{
					_nakiEffect->SetTargetPlayer(player);
					_nakiEffect->PlayEffect(player->GetPos());
				}
			}

			// PlayerMono が検知されたら即時モノ->タヌキに切替 
			if(chaseStarted && _showMonoPlayer && dynamic_cast<PlayerMono*>(player))
			{
				if(_playerTanuki && player != _playerTanuki.get())
				{
					_showMonoPlayer = false;
					_bShowTanuki = true;

					_playerTanuki->SetPos(player->GetPos());
					_playerTanuki->SetDir(player->GetDir());
					_playerTanuki->SetRotationY(atan2f(-player->GetDir().x, -player->GetDir().z));
					_playerTanuki->_status = CharaBase::STATUS::WAIT;
					_playerTanuki->SetMakimonoCount(player->GetMakimonoCount());
					_playerTanuki->PlayAnimation("goepon_idle", true);
					_playerTanuki->Process();

					_hensinEffect->PlayEffect(_playerTanuki->GetPos());
					_walkEffect->SetPlayerPos(_playerTanuki.get());

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
			if(chaseStarted && isHumanForm)
			{
				if(_playerTanuki && player != _playerTanuki.get())
				{
					_showMonoPlayer = false;
					_bShowTanuki = true;

					_playerTanuki->SetPos(player->GetPos());
					_playerTanuki->SetDir(player->GetDir());
					_playerTanuki->SetRotationY(atan2f(-player->GetDir().x, -player->GetDir().z));
					_playerTanuki->_status = CharaBase::STATUS::WAIT;
					_playerTanuki->SetMakimonoCount(player->GetMakimonoCount());
					_playerTanuki->PlayAnimation("idle", true);
					_playerTanuki->Process();
					reEffect = true;

					if(reEffect)
					{
						_hensinEffect->PlayEffect(_playerTanuki->GetPos());
						_walkEffect->SetPlayerPos(_playerTanuki.get());
						//_aseEffect->SetPlayer(_playerTanuki.get());
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
				chaseStarted = false;
			}
		}
	}
	return false;
}


// すべての敵のセンサーをチェックして、プレイヤーが検知されているかどうかを判定する
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

	ProcessEnemyContainer(_enemyBase, player, isHumanForm, anyDetected, reEffect);

	if(!anyDetected && _nakiEffect)
	{
		_nakiEffect->ResetEffect();
		reEffect = false;
	}

	_bTransCancel = anyDetected;

	return anyDetected;
}

void ModeGame::RenderShadowCastersFromModeGame()
{
	// 敵
	for(auto& enemy : _enemyBase)
	{
		if(enemy && enemy->IsAlive())
		{
			enemy->Render();
		}
	}

	// 宝箱
	for(auto& t : _treasureBase)
	{
		if(t)
		{
			t->Render();
		}
	}

	//// 巻物
	//for(auto& m : _makimono)
	//{
	//	if(m)
	//	{
	//		m->Render();
	//	}
	//}

	// プレイヤー（表示中のみ）
	if(_bShowTanuki)
	{
		if(_playerTanuki && _playerTanuki->IsAlive()) { _playerTanuki->Render(); }
	}
	else if(_showMonoPlayer)
	{
		if(_playerMono && _playerMono->IsAlive()) { _playerMono->Render(); }
	}
	else
	{
		if(_player && _player->IsAlive()) { _player->Render(); }
	}
}