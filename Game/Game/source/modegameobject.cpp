/*********************************************************************/
// * \file   modegameobject.cpp
// * \brief  モードゲームクラス(オブジェクト初期化)
// *
// * \author 鈴木裕稀
/*********************************************************************/

#include "modegame.h"
#include "player.h"
//#include "cube.h"
#include "map.h"
#include "PlayerFactory.h"
#include "effectmanager.h"
#include "markermanager.h"
#include "playertanuki.h"
#include "playerform.h"
#include "playermono.h"

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

	// ゴール初期化
	_goal = std::make_shared<Goal>();
	_object.emplace_back(_goal);

	// ui初期化
	_henshinUi = std::make_shared<HenshinUi>();
	_henshinUi->SetOwner(this);
	_uiBase.emplace_back(_henshinUi);
	_uiMakimonoCnt = std::make_shared<UiMakimonoCnt>();
	_uiBase.emplace_back(_uiMakimonoCnt);
	_counterUi = std::make_shared<CounterUi>();
	_uiBase.emplace_back(_counterUi);
	_attackUi = std::make_shared<AttackUi>();
	_uiBase.emplace_back(_attackUi);
	_treasureOpenUi = std::make_shared<TreasureOpenUi>();
	_uiBase.emplace_back(_treasureOpenUi);
	_dashUi = std::make_shared<DashUi>();
	_uiBase.emplace_back(_dashUi);
	_treasureUi = std::make_shared<TreasureUi>();
	_treasureUi->SetTreasureList(_treasureBase);
	_uiBase.emplace_back(_treasureUi);
	_introUi = std::make_shared<IntroUi>();
	_introUi->SetOwner(this);
	_uiBase.emplace_back(_introUi);

	PlayerFactory::Initialize();
	EffectManager::Initialize();
	MarkerManager::Initialize();
	PlayerForm::GetInstance()->Initialize();

	// プレイヤー参照登録
	_playerBase.clear();
	_playerBase.emplace_back(PlayerFactory::GetHumanPlayer());
	_playerBase.emplace_back(PlayerFactory::GetTanukiPlayer());
	_playerBase.emplace_back(PlayerFactory::GetMonoPlayer());



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
		auto* playerForm = PlayerForm::GetInstance();
		PlayerBase* startPlayer = playerForm->GetPlayer();

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
	auto* tanuki = dynamic_cast<PlayerTanuki*>(PlayerFactory::GetTanukiPlayer());
	if(!tanuki)
	{
		return false;
	}

	auto* playerManager = PlayerManager::GetInstance();

	// アニメーション開始
	if(playerManager->GetTransformAnimation() == -1)
	{
		int animId = tanuki->PlayAnimation("henge", false); // 変身アニメーションを再生
		playerManager->SetTransformAnimation(animId);

		if(player)
		{
			playerManager->RequestTransformToHuman(); // 人間への変身要求
		}
		else
		{
			playerManager->RequestTransformToMono(); // モノへの変身要求
		}

		auto henshinSound = gGlobal._soundServer->Get("2");
		if(henshinSound && !henshinSound->IsPlay())
		{
			henshinSound->Play();
		}

		return true; // 変身処理開始
	}

	int transformAnim = playerManager->GetTransformAnimation();
	if(transformAnim != -1 && AnimationManager::GetInstance()->IsPlaying(transformAnim))
	{
		tanuki->Process(); // 変身アニメーションの進行を更新
		return true; // 変身アニメーションが再生中なので処理継続
	}

	playerManager->SetTransformAnimation(-1); // 変身アニメーションIDをリセット

	// 変身アニメーションが終了している場合は、プレイヤーの状態を切り替える
	if(playerManager->IsTransformRequest())
	{
		PlayerBase* player = nullptr;
		if(playerManager->GetPlayerState() == PlayerManager::PlayerState::HUMAN)
		{
			player = PlayerFactory::GetHumanPlayer();
			CompleteTransformToHuman(tanuki, player);
		}
		else if(playerManager->GetPlayerState() == PlayerManager::PlayerState::MONO)
		{
			player = PlayerFactory::GetMonoPlayer();
			CompleteTransformToMono(tanuki, player);
		}
	}

	return true;
}

bool ModeGame::CompleteTransformToHuman(PlayerTanuki* tanuki, PlayerBase* player)
{
	if(!tanuki || !player)
	{
		return false;
	}
	auto* playerManager = PlayerManager::GetInstance();
	auto* playerForm = PlayerForm::GetInstance();

	// タヌキから人間へ変身完了
	player->SetPos(tanuki->GetPos());
	player->SetDir(tanuki->GetDir());
	player->SetRotationY(atan2f(-tanuki->GetDir().x, -tanuki->GetDir().z));

	// プレイヤーを切り替える
	playerForm->ChangeState(PlayerBase::PlayerType::HUMAN);

	// アニメーションを開始
	player->PlayAnimation("idle", true);
	player->_status = CharaBase::STATUS::WAIT;

	EffectManager::UpdatePalyerTransformEffect(player, true);
	EffectManager::UpdatePlayerPosition(player);
	playerManager->SetTransformTimeLimit(17.0f);

	return true;
}

bool ModeGame::CompleteTransformToMono(PlayerTanuki* tanuki, PlayerBase* mono)
{
	if(!tanuki || !mono)
	{
		return false;
	}

	auto* playerManager = PlayerManager::GetInstance();
	auto* playerForm = PlayerForm::GetInstance();

	mono->SetPos(tanuki->GetPos());
	mono->SetDir(tanuki->GetDir());
	mono->SetRotationY(atan2f(-tanuki->GetDir().x, -tanuki->GetDir().z));
	mono->SetMakimonoCount(tanuki->GetMakimonoCount());

	// プレイヤーを切り替える
	playerForm->ChangeState(PlayerBase::PlayerType::MONO);

	// アニメーションを開始
	mono->PlayAnimation("idle", true);
	mono->_status = CharaBase::STATUS::WAIT;

	EffectManager::UpdatePalyerTransformEffect(mono, true);
	EffectManager::UpdatePlayerPosition(mono);

	// 時間制限を設定
	playerManager->SetTransformTimeLimit(12.0f);

	return true;
}

bool ModeGame::RequestTransform(HenshinUi::Select select)
{
	auto* playerForm = PlayerForm::GetInstance();
	PlayerBase* player = playerForm->GetPlayer();

	if(!player)
	{
		return false;
	}

	switch(select)
	{
	case HenshinUi::Select::TANUMONO:
	{
		if(player->GetMakimonoCount() > 0)
		{
			player->SubMakimono(1);
			playerForm->ChangeState(PlayerBase::PlayerType::MONO);
			return true;
		}
		else
		{
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
		if(player->GetMakimonoCount() > 0)
		{
			player->SubMakimono(1);
			playerForm->ChangeState(PlayerBase::PlayerType::HUMAN);
			return true;
		}
		else
		{
			auto soundNoMakimono = gGlobal._soundServer->Get("61");
			if(soundNoMakimono && !soundNoMakimono->IsPlay())
			{
				soundNoMakimono->Play();
			}
			return false;
		}
	}
	default:
		return false;
	}
}

bool ModeGame::UpdateMonoTimeLimit()
{
	auto* playerForm = PlayerForm::GetInstance();
	auto* playerManager = PlayerManager::GetInstance();

	if(playerForm->GetPlayerType() != PlayerBase::PlayerType::MONO || 
		!playerManager->IsTransformTimeLimitActive())
	{
		return false;
	}

	playerManager->UpdateTransformTimer(1.0f / 60.0f);

	if(playerManager->GetTransformTimeLimit() > 0.0f)
	{
		return false;
	}

	// 時間切れ → タヌキに戻す
	playerForm->ChangeState(PlayerBase::PlayerType::TANUKI);
	playerManager->TransformToTanuki();

	return true;
}

bool ModeGame::UpdateHumanTimeLimit()
{
	auto* playerForm = PlayerForm::GetInstance();
	auto* playerManager = PlayerManager::GetInstance();
	if(playerForm->GetPlayerType() != PlayerBase::PlayerType::HUMAN || 
		!playerManager->IsTransformTimeLimitActive())
	{
		return false;
	}
	playerManager->UpdateTransformTimer(1.0f / 60.0f);
	if(playerManager->GetTransformTimeLimit() > 0.0f)
	{
		return false;
	}
	// 時間切れ → タヌキに戻す
	playerForm->ChangeState(PlayerBase::PlayerType::TANUKI);
	playerManager->TransformToTanuki();
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

	auto* playerForm = PlayerForm::GetInstance();
	if(playerForm)
	{
		PlayerBase* activePlayer = playerForm->GetPlayer();
		if(activePlayer && activePlayer->IsAlive())
		{
			activePlayer->Process();
		}
	}

	// 敵（追跡/移動はここで実行される）
	for(auto& enemy : _enemyBase)
	{
		if(enemy->IsAlive())
		{
			// プレイヤーからの距離が遠すぎる敵は思考・移動・アニメーション更新を止める
			auto* playerForm = PlayerForm::GetInstance();
			PlayerBase* player = playerForm->GetPlayer();
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

	auto* playerManager = PlayerManager::GetInstance();

	// 変身リクエストがあればアニメーション開始
	if(playerManager->IsTransformRequest() && !playerManager->IsTransforming())
	{
		playerManager->StartTransformAnimation();
	}

	// アニメーション進行中のプレイヤー処理
	if(playerManager->IsTransforming())
	{
		auto* playerForm = PlayerForm::GetInstance();
		if(playerForm && playerForm->GetPlayerType() == PlayerBase::PlayerType::TANUKI)
		{
			auto* tanuki = dynamic_cast<PlayerTanuki*>(playerForm->GetPlayer());
			if(tanuki)
			{
				tanuki->Process(); // アニメーション進行を更新
			}
		}

		// アニメーション終了確認
		int transformAnim = playerManager->GetTransformAnimation();
		if(transformAnim != -1 && !AnimationManager::GetInstance()->IsPlaying(transformAnim))
		{
			playerManager->CompleteTransform();
		}
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

			auto* playerManager = PlayerManager::GetInstance();
			// 変身時間制限が10秒以下のとき点滅開始
			bool useBlink = (playerManager->IsTransformTimeLimitActive() &&
				playerManager->GetTransformTimeLimit() <= 10.0f);

			int modelHandle = player->GetModelHandle();
			if(modelHandle >= 0)
			{
				int materialNum = MV1GetMaterialNum(modelHandle);

				if(useBlink && !playerManager->GetBlinkVisible())
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

	// プレイヤーの描画
	auto* playerForm = PlayerForm::GetInstance();
	PlayerBase* player = playerForm->GetPlayer();

	if(player && player->IsAlive())
	{
		auto playerType = playerForm->GetPlayerType();
		if(playerType == PlayerBase::PlayerType::TANUKI)
		{
			// タヌキは点滅なしで描画
			player->Render();
		}
		else
		{
			// 人間とモノは点滅ロジックを適用
			renderPlayerWithBlink(player);
		}
	}

	// UIが参照するプレイヤーを「現在表示中」に合わせる
	if(_uiHp)
	{
		_uiHp->SetPlayer(player);
	}
	if(_uiMakimonoCnt)
	{
		_uiMakimonoCnt->SetPlayer(player);
	}
	if(_dashUi)
	{
		if(playerForm->GetPlayerType() == PlayerBase::PlayerType::TANUKI)
		{
			_dashUi->SetPlayer(player);
		}
		else
		{
			_dashUi->SetPlayer(nullptr);
		}
	}

	// ---- ここから先は「演出/ゲージ/UI」なので必ず影なしにする ----
	SetUseLighting(FALSE);

	// Effekseer（3D/2D）を影なしで描画（ワールドの後、UIの前）
	EffekseerManager::GetInstance()->Render();

	// 各敵のセンサーを個別に描画
	// プレイヤーから半径内にいる敵だけ索敵範囲を描画
	if(player)
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
			vec::Vec3 vecToPlayer = vec3::VSub(enemy->GetPos(), player->GetPos());
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

	if(player)
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

				treasureBase->RenderGauge(player->GetPos(), treasureProgress);
			}
		}

		// 連打宝箱のゲージ描画
		for(const auto& treasureRapidFire : _treasureRapidFire)
		{
			if(treasureRapidFire && treasureRapidFire->IsVisible() && !treasureRapidFire->IsOpen())
			{
				treasureRapidFire->RenderGaugeRF(player->GetPos(), 0.0f);
			}
		}
	}

	SetUseZBuffer3D(FALSE);
	SetWriteZBuffer3D(FALSE);

	for(auto& ui_base : _uiBase)
	{
		ui_base->Render();
	}

	// 状態復帰（次の描画に影響を残さない）
	SetUseLighting(TRUE);

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
	auto* playerForm = PlayerForm::GetInstance();
	auto* playerManager = PlayerManager::GetInstance();

	if(container.empty() || !player)
	{
		return false;
	}

	constexpr float kMonoMoveDetectThreshold = 0.1f;
	static size_t s_nextProcessIndex = 0;

	int nonChasingProcessedCount = 0;
	const int kMaxNormalChecksPerFrame = 1;

	for(size_t i = 0; i < container.size(); ++i)
	{
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

		sensor->SetPos(eb->GetPos());
		sensor->SetDir(eb->GetDir());
		sensor->SetMap(_objectServer->GetMap());

		const bool wasChasing = sensor->IsChasing();

		if(!wasChasing)
		{
			const float activeRadius = 1000.0f;
			vec::Vec3 vecToEnemy = vec3::VSub(eb->GetPos(), player->GetPos());
			vecToEnemy.y = 0.0f;
			if(vec3::VSize(vecToEnemy) > activeRadius)
			{
				continue;
			}

			if(nonChasingProcessedCount >= kMaxNormalChecksPerFrame)
			{
				continue;
			}

			nonChasingProcessedCount++;
			s_nextProcessIndex = (currentIndex + 1) % container.size();
		}

		sensor->Process();

		bool detected = false;
		bool chaseStarted = false;
		bool isEnemyDog = (dynamic_cast<EnemyDog*>(eb) != nullptr);

		// PlayerMono 表示時の特別処理
		if(playerForm->GetPlayerType() == PlayerBase::PlayerType::MONO &&
			dynamic_cast<PlayerMono*>(player))
		{
			vec::Vec3 playerPos = player->GetPos();
			vec::Vec3 capsuleTop = vec3::VAdd(playerPos, vec3::VGet(0.0f, player->GetColSubY(), 0.0f));
			vec::Vec3 capsuleBottom = vec3::VAdd(playerPos, vec3::VGet(0.0f, -player->GetColSubY(), 0.0f));
			float capsuleRadius = player->GetCollisionR();

			if(sensor->IsPlayerInDetectionRangeWithCapsule(playerPos, capsuleTop, capsuleBottom, capsuleRadius))
			{
				vec::Vec3 delta = vec3::VSub(player->GetPos(), player->GetOldPos());
				float moved = vec3::VSize(delta);
				bool inputMoving = (vec3::VSize(player->GetInputVector()) > 0.001f);

				if(moved > kMonoMoveDetectThreshold || inputMoving)
				{
					detected = sensor->CheckPlayerDetection(player);
				}
				else
				{
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
			detected = sensor->CheckPlayerDetection(player);
		}
		else
		{
			if(player != nullptr && sensor != nullptr)
			{
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

		if(detected)
		{
			anyDetected = true;
			if(player != nullptr)
			{
				eb->OnPlayerDetected(player->GetPos());
				if(playerForm->GetPlayerType() == PlayerBase::PlayerType::TANUKI)
				{
					// hatenaeffect を再生する
					auto hatenaEffect = EffectManager::GetHatenaEffect();
					if(hatenaEffect)
					{
						hatenaEffect->PlayOnce(eb);
					}
				}
				else
				{
					// 通常形態の場合は既存の処理
					EffectManager::GetHatenaEffect()->ResetEnemyEffect(eb);
				}

				const bool isChasingNow = sensor->IsChasing();
				chaseStarted = (!wasChasing && isChasingNow);

				if(eb->GetEnemySensor() && eb->GetEnemySensor()->IsChasing())
				{
					EffectManager::GetNakiEffect()->SetTargetPlayer(player);
					EffectManager::GetNakiEffect()->PlayEffect(player->GetPos());
				}
			}

			// PlayerMono が検知されたら即時モノ->タヌキに切替 
			if(chaseStarted && playerForm->GetPlayerType() == PlayerBase::PlayerType::MONO &&
				dynamic_cast<PlayerMono*>(player))
			{
				playerManager->RequestTransformToTanuki();
				playerForm->TransformForEnemyDetetion(
					PlayerBase::PlayerType::TANUKI,
					player->GetPos(),
					player->GetDir()
				);
			}

			// 人状態で尻尾（後方）を見られた場合、強制的にタヌキ表示へ切替
			if(chaseStarted && isHumanForm)
			{
				playerManager->RequestTransformToTanuki();
				playerForm->TransformForEnemyDetetion(
					PlayerBase::PlayerType::TANUKI,
					player->GetPos(),
					player->GetDir()
				);
			}
		}
		else
		{
			if(!sensor->IsChasing())
			{
				if(eb->IsDetectPlayer())
				{
					EffectManager::GetHatenaEffect()->ResetEnemyEffect(eb);
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
	auto* playerForm = PlayerForm::GetInstance();
	PlayerBase* player = playerForm->GetPlayer();

	if(!player)
	{
		return false;
	}

	bool isHumanForm = (playerForm->GetPlayerType() == PlayerBase::PlayerType::HUMAN);

	bool anyDetected = false;
	bool reEffect = false;

	ProcessEnemyContainer(_enemyBase, player, isHumanForm, anyDetected, reEffect);

	if(!anyDetected)
	{
		EffectManager::GetNakiEffect()->ResetEffect();
	}

	_bTransCancel = anyDetected;

	return anyDetected;
}

void ModeGame::RenderShadowCastersFromModeGame()
{
	auto* playerForm = PlayerForm::GetInstance();

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

	// プレイヤー（表示中のみ）
	PlayerBase* player = playerForm->GetPlayer();
	if(player && player->IsAlive())
	{
		player->Render();
	}
}