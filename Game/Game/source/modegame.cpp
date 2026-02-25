/*********************************************************************/
// * \file   modegame.cpp
// * \brief  モードゲームクラス
// *
// * \author 鈴木裕稀
// * \date   2025/12/15
// * \作業内容	: 新規作成 鈴木裕稀　2025/12/15
//				: UI HP追加	鈴木裕稀 2026/01/06
//				vec::Vec3を使用するように修正　鈴木裕稀　2026/01/17
/*********************************************************************/

#include "modegame.h"
#include "applicationmain.h"
#include "modeeffekseer.h"
#include "ModeGameClear.h"
#include "applicationglobal.h"
#include "ModeGameOver.h"

#ifdef _DEBUG
#include <crtdbg.h>
#define NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
#else
#define NEW new
#endif


// 初期化
bool ModeGame::Initialize()
{
	_hasRenderOnce = false;
	_requestResetStage = false;


	if(!base::Initialize()) { return false; }

	ObjectInitialize();	// オブジェクト初期化

	// オブジェクトサーバー初期化
	/*_objectServer = NEW ObjectServer(this);
	_objectServer->LoadDate("SM_stagebeta");
	_objectServer->ProcessInit();*/
	_objectServer = ApplicationMain::GetInstance()->GetObjectServer();

	// 3Dサウンドサーバーの初期化（applicationglobal から取得）
	// gGlobal._soundServer は ApplicationGlobal::Init() 内で生成されている想定
	if(gGlobal._soundServer)
	{
		_sound3D = std::make_shared<SoundServer3D>(gGlobal._soundServer);
		// デフォルトの半径を設定（必要に応じて調整）
		_sound3D->SetRadius(768.0f);
	}
	else
	{
		_sound3D.reset();
	}

	LoadStageData();// ステージデータ読み込み

	// ゴール初期化
	_isGameClear = false;

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

	// カメラセット
	if(auto* map = _objectServer->GetMap())
	{
		map->SetCamera(_camera);
	}
	_bShowTanuki = true;

	_player->SetCamera(_camera);
	_playerTanuki->SetCamera(_camera);
	_playerMono->SetCamera(_camera);
	_treasureEffect->SetTreasure(_treasure);
	_walkEffect->SetPlayerPos(_playerTanuki.get());
	_findEffect->SetEnemy(_enemyBase);
	_aseEffect->SetPlayer(_playerTanuki.get());
	_hatenaEffect->Enemy(_enemyBase);
	_aseEffect->SetEnemy(_enemyBase);

	DebugInitialize();// デバック初期化
	ShadowInitialize();// シャドウ生成
	CameraInfoInitialize();// カメラ情報初期化

	_bResolveOnY = false;
	_bLandedOnUp = false;
	_bCameraControlMode = false;
	_hasSavedCameraState = false;

	_isChengeBgm = false;

	// BGM再生
	_bgmInitialize = gGlobal._soundServer->Get("bgminitialize");
	_bgmChenge = gGlobal._soundServer->Get("bgmChenge");
	_bgmInitialize->Play();

	//// **ロード時間計測終了**
	//const LONGLONG endTime = GetNowHiPerformanceCount();
	//_loadTimeMs = static_cast<float>(endTime - startTime) / 1000.0f; // ミリ秒に変換

	_stageManager.SetStages
	({
	   "stage01",
	   "stage02",
	   "stage03",
	});

	return true;
}

// 終了
bool ModeGame::Terminate()
{
	base::Terminate();
	// キャラ
	for(auto& chara : _chara)
	{
		chara->Terminate();
	}
	_chara.clear();
	for(auto& object : _object)
	{
		object->Terminate();
	}
	_object.clear();
	for(auto& player_base : _playerBase)
	{
		player_base->Terminate();
	}
	_playerBase.clear();
	for(auto& ui_base : _uiBase)
	{
		ui_base->Terminate();
	}
	_uiBase.clear();

	for(auto& charaShadow : _charaShadow)
	{
		charaShadow->Terminate();
	}
	_charaShadow.clear();
	// エフェクト
	for(auto& effectBase : _effectBase)
	{
		effectBase->Terminate();
	}
	_effectBase.clear();

	delete _camera;

	// 索敵システムの終了処理
	if(_enemySensor)
	{
		_enemySensor->Terminate();
		_enemySensor.reset();
	}
	if(_sound3D) {
		_sound3D->StopAll();
		_sound3D.reset();
	}

	//if(_modeGameLoad)
	//{
	//	ModeServer::GetInstance()->Del(_modeGameLoad);
	//	_modeGameLoad = nullptr;
	//}

	return true;
}

bool ModeGame::LoadStageData()
{

	const ApplicationGlobal::StageData* stageData = gGlobal.GetStageData("Stage1");
	if(stageData == nullptr)
	{
		return false;
	}

	const auto& patrolGroup = stageData->patrolGroup;
	const auto& objectList = stageData->object;

	// **最適化: メモリ予約で再割り当てを削減**
	const size_t objCount = objectList.size();
	_enemyBase.reserve(objCount / 4);
	_treasure.reserve(objCount / 8);
	_makimono.reserve(objCount / 8);

	// 敵の JSON を一時保存して後で巡回グループを割り当てる
	std::vector<nlohmann::json> enemyObjects;
	enemyObjects.reserve(objCount / 4);

	uint32_t nextEnemyId = 1;

	// **修正: 二重ループを削除し、1回のループで全オブジェクトを処理**
	for(auto&& objData : stageData->object)
	{
		const std::string& name = objData.objectName;
		const nlohmann::json& object = objData.json;

		// S_MarkerRは既にpatrolGroupsに格納済みなのでスキップ
		if(name == "S_MarkerR")
		{
			continue;
		}

		if(name == "S_MarkerA")
		{
			_playerTanuki->SetJsonDataUE(object);
			continue;
		}

		// 敵は一旦保留（後でcustomIdに対応した巡回点を割り当てる）
		if(name == "S_MarkerB" || name == "S_MarkerRX" || name == "Dog")
		{
			enemyObjects.push_back(object);
			continue;
		}

		if(name == "Goal")
		{
			_goal->SetJsonDataUE(object);
			continue;
		}

		if(name == "Treasure")
		{
			auto treasure = std::make_shared<Treasure>();
			treasure->Initialize();
			treasure->SetJsonDataUE(object);
			_treasure.emplace_back(treasure);
			continue;
		}

		if(name == "Item")
		{
			auto makimono = std::make_shared<Makimono>();
			makimono->Initialize();
			makimono->SetJsonDataUE(object);
			makimono->SetCamera(_camera);
			_makimono.emplace_back(makimono);
			continue;
		}
	}

	// **最適化: センサー生成を共通化するヘルパー関数**
	auto* mapPtr = _objectServer->GetMap();
	auto createSensors = [mapPtr](float soundArea) -> std::pair<std::shared_ptr<EnemySensor>, std::shared_ptr<EnemySoundSensor>>
		{
			auto sensor = std::make_shared<EnemySensor>();
			sensor->Initialize();
			sensor->SetMap(mapPtr);

			auto soundSensor = std::make_shared<EnemySoundSensor>();
			soundSensor->Initialize();
			soundSensor->SetMap(mapPtr);
			soundSensor->SetSoundSensorArea(soundArea);

			return { sensor, soundSensor };
		};

	// 敵を生成して、customIdにマッチする巡回点を割り当てる
	for(auto& object : enemyObjects)
	{
		const std::string& name = object.at("objectName");

		// customIdを取得（無ければ空文字""を使う）
		std::string gid;
		if(object.contains("customId"))
		{
			object.at("customId").get_to(gid);
		}

		if(name == "S_MarkerRX")
		{
			auto [sensor, soundSensor] = createSensors(300.0f);

			auto enemy = std::make_shared<Enemy>();
			enemy->Initialize();
			enemy->SetJsonDataUE(object);
			enemy->SetEnemySensor(sensor);
			enemy->SetEnemySoundSensor(soundSensor);
			soundSensor->SetPos(enemy->GetPos());
			enemy->SetEffect(_hensinEffect);
			enemy->SetEnemyId(nextEnemyId++);
			enemy->SetDirSequenceFromJson(object);

			_enemyBase.emplace_back(enemy);
			continue;
		}

		if(name == "S_MarkerB")
		{
			auto [sensor, soundSensor] = createSensors(300.0f);

			auto enemyMove = std::make_shared<EnemyMove>();
			enemyMove->Initialize();
			enemyMove->SetJsonDataUE(object);
			enemyMove->SetEnemySensor(sensor);
			enemyMove->SetEnemySoundSensor(soundSensor);
			soundSensor->SetPos(enemyMove->GetPos());
			enemyMove->SetEffect(_hensinEffect);
			enemyMove->SetEnemyId(nextEnemyId++);
			enemyMove->SetDirSequenceFromJson(object);

			// グループに対応する巡回点があれば割り当てる
			auto it = patrolGroup.find(gid);
			if(it != patrolGroup.end() && !it->second.empty())
			{
				enemyMove->SetPatrolPoint(it->second);
				enemyMove->CaptureInitialTransform();
			}

			_enemyBase.emplace_back(std::move(enemyMove));
			continue;
		}

		if(name == "Dog")
		{
			auto [sensor, soundSensor] = createSensors(900.0f);

			auto enemyDog = std::make_shared<EnemyDog>();
			enemyDog->Initialize();
			enemyDog->SetJsonDataUE(object);
			enemyDog->SetEnemySensor(sensor);
			enemyDog->SetEnemySoundSensor(soundSensor);
			soundSensor->SetPos(enemyDog->GetPos());
			enemyDog->SetEffect(_hensinEffect);
			enemyDog->SetEnemyId(nextEnemyId++);

			_enemyBase.emplace_back(std::move(enemyDog));
		}
	}

	// 互換のため全敵に初期トランスフォームを確実にキャプチャ
	for(auto&& enemy : _enemyBase)
	{
		if(enemy) enemy->CaptureInitialTransform();
	}

	return true;
}

// 円同士の当たり判定
bool ModeGame::IsHitCircle(CharaBase* c1, CharaBase* c2)
{
	// 「2つの円の中心点」の距離を求める
	// ピタゴラスの定理（三平方の定理）で求めることができる
	// x,zで処理。yは判定せず
	float w, h, length;
	w = c1->GetPos().x - c2->GetPos().x;
	h = c1->GetPos().z - c2->GetPos().z;
	length = StCas<float>(sqrtf(w * w + h * h));

	// 中心点間の距離が、2つの円の半径の合計よりも小さい場合、当たり
	if(length < c1->GetCollisionR() + c2->GetCollisionR())
	{
		return true;
	}

	return false;
}

// プレイヤーのカメラ情報表示
bool ModeGame::PlayerCameraInfo(PlayerBase* player)
{
	// カメラの位置/視点の移動を、プレイヤーの移動量に追従する
	vec::Vec3 playermove = vec3::VSub(player->GetPos(), player->GetOldPos());
	_camera->_vPos = vec3::VAdd(_camera->_vPos, playermove);
	_camera->_vTarget = vec3::VAdd(_camera->_vTarget, playermove);
	return true;
}

// 計算処理
bool ModeGame::Process()
{
	base::Process();

	//// **ロード完了後の最初のフレームでロード画面を削除**
	//if(!_isLoadComplete)
	//{
	//	_isLoadComplete = true;
	//	if(_modeGameLoad)
	//	{
	//		ModeServer::GetInstance()->Del(_modeGameLoad);
	//		_modeGameLoad = nullptr;
	//	}
	//	return true; // 最初のフレームは他の処理をスキップ
	//}


	// ★クリア画面が消えた後にここが回り始める想定なので、ここで実行するのが安全
	//if(_requestResetStage)
	//{
	//	_requestResetStage = false;
	//	ResetStage();
	//	return true;
	//}

	ModeServer::GetInstance()->SkipProcessUnderLayer();
	ModeServer::GetInstance()->SkipRenderUnderLayer();

	if(_requestResetStage)
	{
		_requestResetStage = false;
		ResetStage();
		return true;
	}

	// カメラ処理
	_camera->Process();

	DebugProcess();
	DebugCameraControl();

	//_soundServer->Update();
	// アニメーション処理

	AnimationManager::GetInstance()->Update(1.0f);

	// Effekseer 更新

	EffekseerManager::GetInstance()->Update();

	// 敵サウンド処理
	EnemySoundManager::GetInstance()->Update(1.0f / 60.0f);

	// 敵の音検出処理
	for(auto& e : _enemyBase)
	{
		if(!e || !e->IsAlive())
		{
			continue;
		}

		EnemySoundManager::DetectionInfo info{};
		if(EnemySoundManager::GetInstance()->TryDetectForEnemy(*e, info) && info.isDetected)
		{
			// ここを「EnemyMove だけ」ではなく「全敵」に反映する
			if(auto* moveEnemy = dynamic_cast<EnemyMove*>(e.get()))
			{
				moveEnemy->StartMoveToSound(info.soundSourcePos, info.detectedSoundLevel);
			}
			else
			{
				e->StartMoveToSoundFromManager(info.soundSourcePos, info.detectedSoundLevel);
			}
		}
	}


	// オブジェクトサーバー処理
	_objectServer->ProcessInit(); // 追加・削除予約の確定
	_objectServer->Process();


	// 3Dサウンドリスナー位置の設定（毎フレーム更新）
	PlayerBase* activePlayer = nullptr;
	if(_bShowTanuki)
	{
		activePlayer = _playerTanuki.get();
	}
	else if(_showMonoPlayer)
	{
		activePlayer = _playerMono.get();
	}
	else
	{
		activePlayer = _player.get();
	}

	if(activePlayer)
	{
		vec::Vec3 listenerPos = activePlayer->GetPos();
		vec::Vec3 listenerFront = vec3::VAdd(listenerPos, activePlayer->GetDir());
		Set3DSoundListenerPosAndFrontPos_UpVecY(
			DxlibConverter::VecToDxLib(listenerPos),
			DxlibConverter::VecToDxLib(listenerFront)
		);
	}

	// プレイヤー変身処理
	PlayerTransform();
	// オブジェクト処理
	ObjectProcess();

	// 敵との当たり判定処理（生存している敵のみ）
	// 	...
	// 当たり判定の処理をここに書く

	if(_d_use_collision)
	{
		CheckAllDetections();
	}

	// 敵AI（追跡/移動はここで実行される）

	if(_bShowTanuki)
	{
		EscapeCollision(_playerTanuki.get(), _objectServer->GetMap());
		const bool hitTreasure = CharaToTreasureHitCollision(_playerTanuki.get(), _treasure);
		CharaToTreasureOpenCollision(_playerTanuki.get(), _treasure);
		PlayerCameraInfo(_playerTanuki.get());
	}
	else if(_showMonoPlayer)
	{
		EscapeCollision(_playerMono.get(), _objectServer->GetMap());
		const bool hitTreasure = CharaToTreasureHitCollision(_playerMono.get(), _treasure);
		CharaToTreasureOpenCollision(_playerMono.get(), _treasure);
		PlayerCameraInfo(_playerMono.get());
	}
	else
	{
		EscapeCollision(_player.get(), _objectServer->GetMap());
		const bool hitTreasure = CharaToTreasureHitCollision(_player.get(), _treasure);
		CharaToTreasureOpenCollision(_player.get(), _treasure);
		PlayerCameraInfo(_player.get());
	}

	PlayerBase* playerBase = nullptr;

	// 表示中のプレイヤーを明示的に選択（タヌキ / Mono / 通常）
	if(_bShowTanuki)
	{
		playerBase = _playerTanuki.get();
	}
	else if(_showMonoPlayer)
	{
		playerBase = _playerMono.get();
	}
	else
	{
		playerBase = _player.get();
	}

	// ここで呼ぶ（playerBase が確定してから）
	PlayerToMakimonoCollision(playerBase, _makimono);

	// プレイヤーと敵の接触処理
	if(playerBase && playerBase->IsAlive())
	{
		for(auto& enemy : _enemyBase)
		{
			if(!enemy || !enemy->IsAlive())
			{
				continue;
			}

			// 軽量な早期判定（XZ円）
			if(CharaToCharaCollision(playerBase, enemy.get()))
			{
				// プレイヤーが敵に見つかっていなかったら捕まらない
				if(enemy->IsPlayerChasing())
				{

					if(gGlobal._soundServer)
					{
						gGlobal._soundServer->StopType(soundserver::SoundItemBase::TYPE::SE);
						gGlobal._soundServer->StopType(soundserver::SoundItemBase::TYPE::VOICE);
						gGlobal._soundServer->StopType(soundserver::SoundItemBase::TYPE::ONESHOT);
						gGlobal._soundServer->StopType(soundserver::SoundItemBase::TYPE::BGM);

						// 明示的にキーで登録されているループ音を停止
						auto walkSound = gGlobal._soundServer->Get("1");
						if(walkSound && walkSound->IsPlay()) walkSound->Stop();
						auto walkSound11 = gGlobal._soundServer->Get("11");
						if(walkSound11 && walkSound11->IsPlay()) walkSound11->Stop();

						// 保持しているBGMハンドルがあるなら停止
						if(_bgmInitialize && _bgmInitialize->IsPlay()) _bgmInitialize->Stop();
						if(_bgmChenge && _bgmChenge->IsPlay()) _bgmChenge->Stop();
					}

					// モード内サウンドサーバーがあれば停止（存在するなら）
					if(_soundServer)
					{
						_soundServer->StopType(soundserver::SoundItemBase::TYPE::SE);
						_soundServer->StopType(soundserver::SoundItemBase::TYPE::VOICE);
						_soundServer->StopType(soundserver::SoundItemBase::TYPE::ONESHOT);
						_soundServer->StopType(soundserver::SoundItemBase::TYPE::BGM);
					}

					// 3Dサウンドの停止
					if(_sound3D)
					{
						_sound3D->StopAll();
					}

					for(auto& effectBase : _effectBase)
					{
						if(effectBase)
						{
							effectBase->StopPlaying();
						}
					}

					//ここでゲームオーバー処理へ移行
					ModeServer::GetInstance()->Add(NEW ModeGameOver(this), 255, "ModeGameOver");

					return true;
				}
				// 実際の押し出し（カプセル）
				// 敵に接触したときに実際に行う処理はここで記入
				// デバッグ用：メッセージ表示
				if(!enemy->IsShowingYouDiedMessage())
				{
					enemy->TriggerYouDiedMessage();
				}
			}
		}
	}

	// デバック用タイマー（転ばせる）
	if(_showKnockdownMessage)
	{
		const float dt = 1.0f / 60.0f; // 60FPS想定
		_knockdownMessageSec -= dt;
		if(_knockdownMessageSec <= 0.0f)
		{
			_showKnockdownMessage = false;
			_knockdownMessageSec = 0.0f;
		}
	}

	// ゴールとの当たり判定
	if(CanGoal() && !_isGameClear)
	{
		// いま操作/表示しているプレイヤーで判定
		PlayerBase* goalPlayer = nullptr;

		// ゴール判定を行うプレイヤーを明示的に選択（タヌキ / Mono / 通常）
		if(_bShowTanuki)
		{
			goalPlayer = _playerTanuki.get();
		}
		else if(_showMonoPlayer)
		{
			goalPlayer = _playerMono.get();
		}
		else
		{
			goalPlayer = _player.get();
		}

		// ゴール判定を行うプレイヤーが有効なら当たり判定をチェック
		UpdateGoalConfirm(goalPlayer);
	}

	// 攻撃判定は「表示中のプレイヤーが人間プレイヤー(_player) のときのみ」実行する
	if(playerBase == _player.get())
	{
		IsPlayerAttack(_player.get(), _enemyBase);
	}
	

	// 3Dサウンド処理

	if(_sound3D)
	{
		// 全ての EnemyBase 系に対して、歩行中のみ3D音を再生する
		for(auto& eb : _enemyBase)
		{
			if(!eb) continue;

			// 生存チェック
			if(eb->IsAlive())
			{
				// 歩行状態のときだけ再生、それ以外は停止
				if(eb->_status == CharaBase::STATUS::WALK)
				{
					// EnemyMove 型かどうかで音を切り替える
					if(dynamic_cast<EnemyMove*>(eb.get()) != nullptr)
					{
						_sound3D->PlayLoopSound3D(eb.get(), "31", eb->GetPos());
					}
					else
					{
						_sound3D->PlayLoopSound3D(eb.get(), "31", eb->GetPos());
					}
				}
				else
				{
					// 歩行していなければループ音を停止
					_sound3D->StopSound3D(eb.get());
				}
			}
			else
			{
				// 死亡時は確実に停止
				_sound3D->StopSound3D(eb.get());
			}
		}
	}

	// 変身時間制限処理
	if(_changeTimeActive)
	{
		float dt = 1.0f / 60.0f; // 60FPS想定
		_changeTimeLimit -= dt;
		// 変更箇所: changeTimeLimit <= 0.0f のブロック内
		if(_changeTimeLimit <= 0.0f)
		{
			_changeTimeActive = false;
			_changeTimeLimit = 0.0f;
			_changeBlinkTimer = 0.0f;
			_changeBlinkVisible = true;

			// 変身解除前に「直前に表示されていたプレイヤー」を取得しておく
			PlayerBase* prevActive = nullptr;
			if(_bShowTanuki)
			{
				prevActive = _playerTanuki.get();
			}
			else if(_showMonoPlayer)
			{
				prevActive = _playerMono.get();
			}
			else
			{
				prevActive = _player.get();
			}

			// タヌキ表示へ切替
			_bShowTanuki = true;
			_showMonoPlayer = false;

			// prevActive が有効ならその位置／向きをタヌキに引き継ぐ
			if(prevActive && _playerTanuki)
			{
				_playerTanuki->SetPos(prevActive->GetPos());
				_playerTanuki->SetDir(prevActive->GetDir());
				_playerTanuki->_status = CharaBase::STATUS::WAIT;
				_playerTanuki->PlayAnimation("idle", true);
				_playerTanuki->Process(); // 変身直後の1フレーム更新
			}

			// カメラを新しいプレイヤー位置に合わせる（オフセットは維持）
			if(_camera && _playerTanuki)
			{
				vec::Vec3 camDelta = vec3::VSub(_camera->_vPos, _camera->_vTarget);
				vec::Vec3 target = vec3::VAdd(_playerTanuki->GetPos(), vec3::VGet(0.0f, 60.0f, 0.0f));
				_camera->_vTarget = target;
				_camera->_vPos = vec3::VAdd(target, camDelta);
			}

			// エフェクト再設定
			_hensinEffect->PlayEffect(_playerTanuki->GetPos());
			_walkEffect->SetPlayerPos(_playerTanuki.get());
			_aseEffect->SetPlayer(_playerTanuki.get());

			auto soundFinish = gGlobal._soundServer->Get("3");
			if(soundFinish && !soundFinish->IsPlay())
			{
				soundFinish->Play();
			}

			// 周囲の敵に対する音波／エフェクト
			for(auto& enemy : _enemyBase)
			{
				if(enemy && enemy->IsAlive())
				{
					enemy->GetSoundSensor()->TriggerSoundWave(_playerTanuki->GetPos(), 500.0f, 10.0f);
					enemy->GetSoundSensor()->SetSoundLevel(5);
					_hatenaEffect->PlayOnce(enemy.get());
				}
			}
		}
		else if(_changeTimeLimit <= 10.0f)
		{
			_changeBlinkTimer += dt;
			if(_changeBlinkTimer >= _changeBlinkInterval)
			{
				_changeBlinkTimer = 0.0f;
				_changeBlinkVisible = !_changeBlinkVisible;
			}
		}
	}

	// BGM変更処理

	ChangeBGM();

	return true;
}

// 描画処理
bool ModeGame::Render()
{
	base::Render();

	// カメラ設定更新
	SetCameraPositionAndTarget_UpVecY(DxlibConverter::VecToDxLib(_camera->_vPos), DxlibConverter::VecToDxLib(_camera->_vTarget));
	SetCameraNearFar(_camera->_fClipNear, _camera->_fClipFar);
	float fov_deg = 30.0f;
	float fov_rad = DEG2RAD(fov_deg);
	SetupCamera_Perspective(fov_rad);

	EffekseerManager::GetInstance()->Render();

	// オブジェクトサーバーの描画
	_objectServer->Render();

	ObjectRender();// オブジェクト描画処理
	//DebugRender(); // デバック描画処理

	// 処理時間を画面に表示
	int y = 40;
	const int lineHeight = 18;
	const unsigned int colorYellow = GetColor(255, 255, 0);
	const unsigned int colorWhite = GetColor(255, 255, 255);
	const unsigned int colorRed = GetColor(255, 100, 100);

	//DrawFormatString(10, y, colorYellow, "=== Performance Monitor ==="); y += lineHeight;

	//// 処理時間が0.5ms以上の項目は赤色で表示
	//DrawFormatString(10, y, _processTotalMs >= 0.5f ? colorRed : colorYellow, "Total Process: %.3f ms", _processTotalMs); y += lineHeight;
	//DrawFormatString(10, y, _processCameraMs >= 0.5f ? colorRed : colorWhite, "  Camera: %.3f ms", _processCameraMs); y += lineHeight;
	//DrawFormatString(10, y, _processAnimationMs >= 0.5f ? colorRed : colorWhite, "  Animation: %.3f ms", _processAnimationMs); y += lineHeight;
	//DrawFormatString(10, y, _processEffekseerMs >= 0.5f ? colorRed : colorWhite, "  Effekseer: %.3f ms", _processEffekseerMs); y += lineHeight;
	//DrawFormatString(10, y, _processEnemySoundMs >= 0.5f ? colorRed : colorWhite, "  EnemySound: %.3f ms", _processEnemySoundMs); y += lineHeight;
	//DrawFormatString(10, y, _processObjectServerMs >= 0.5f ? colorRed : colorWhite, "  ObjectServer: %.3f ms", _processObjectServerMs); y += lineHeight;
	//DrawFormatString(10, y, _processSoundListenerMs >= 0.5f ? colorRed : colorWhite, "  SoundListener: %.3f ms", _processSoundListenerMs); y += lineHeight;
	//DrawFormatString(10, y, _processPlayerTransformMs >= 0.5f ? colorRed : colorWhite, "  PlayerTransform: %.3f ms", _processPlayerTransformMs); y += lineHeight;
	//DrawFormatString(10, y, _processObjectProcessMs >= 0.5f ? colorRed : colorWhite, "  ObjectProcess: %.3f ms", _processObjectProcessMs); y += lineHeight;
	//DrawFormatString(10, y, _processCollisionMs >= 0.5f ? colorRed : colorWhite, "  Collision: %.3f ms", _processCollisionMs); y += lineHeight;
	//DrawFormatString(10, y, _processEnemyAIMs >= 0.5f ? colorRed : colorWhite, "  EnemyAI: %.3f ms", _processEnemyAIMs); y += lineHeight;
	//DrawFormatString(10, y, _processPlayerCollisionMs >= 0.5f ? colorRed : colorWhite, "  PlayerCollision: %.3f ms", _processPlayerCollisionMs); y += lineHeight;
	//DrawFormatString(10, y, _processPlayerEnemyMs >= 0.5f ? colorRed : colorWhite, "  PlayerEnemy: %.3f ms", _processPlayerEnemyMs); y += lineHeight;
	//DrawFormatString(10, y, _processGoalMs >= 0.5f ? colorRed : colorWhite, "  Goal: %.3f ms", _processGoalMs); y += lineHeight;
	//DrawFormatString(10, y, _processAttackMs >= 0.5f ? colorRed : colorWhite, "  Attack: %.3f ms", _processAttackMs); y += lineHeight;
	//DrawFormatString(10, y, _process3DSoundMs >= 0.5f ? colorRed : colorWhite, "  3DSound: %.3f ms", _process3DSoundMs); y += lineHeight;
	//DrawFormatString(10, y, _processChangeTimeMs >= 0.5f ? colorRed : colorWhite, "  ChangeTime: %.3f ms", _processChangeTimeMs); y += lineHeight;
	//DrawFormatString(10, y, _processBGMMs >= 0.5f ? colorRed : colorWhite, "  BGM: %.3f ms", _processBGMMs); y += lineHeight;
	//DrawFormatString(10, y, _processSectorDetectionMs >= 0.5f ? colorRed : colorWhite, "  SectorDetection: %.3f ms", _processSectorDetectionMs); y += lineHeight;

	if(_d_view_collision)
	{
		//CollisionManager::GetInstance()->SetDebugDraw(true);
	}

	_hasRenderOnce = true;

	return true;
}

bool ModeGame::UpdateGoalConfirm(PlayerBase* player)
{
	// プレイヤーがいて、未クリア状態で、ゴールに触れているか
	bool hitGoal = (!_isGameClear && player && PlayerToGoalHitCollision(player, _goal.get()));

	// ゴールから離れたら抑制解除（＝次に踏んだらまた確認OK）
	if(!hitGoal)
	{
		_notGoalFlag = false;
	}

	// 抑制中は確認を開かない（No直後に乗りっぱなしでも再表示しない）
	if(_notGoalFlag)
	{
		return false;
	}

	// 踏んだ瞬間に確認モードを開く
	if(hitGoal && !_goalConfirmOpened)
	{
		_goalConfirmOpened = true;
		_goalConfirmResult = ModeGoalConfirm::Result::None;

		ModeServer::GetInstance()->Add(NEW ModeGoalConfirm(&_goalConfirmResult), 256, "ModeGoalConfirm");
		return true;
	}

	// 確認結果を受けて処理を行う
	if(_goalConfirmOpened)
	{
		if(_goalConfirmResult == ModeGoalConfirm::Result::Yes)
		{
			_goalConfirmOpened = false;
			_goalConfirmResult = ModeGoalConfirm::Result::None;

			_isGameClear = true;
			ModeServer::GetInstance()->Add(NEW ModeGameClear(this), 255, "ModeGameClear");
			return true;
		}
		if(_goalConfirmResult == ModeGoalConfirm::Result::No)
		{
			_goalConfirmOpened = false;
			_goalConfirmResult = ModeGoalConfirm::Result::None;

			// 抑制フラグを立てる（ゴールから離れたら解除される）
			_notGoalFlag = true;
		}

	}

	return false;
}

bool ModeGame::ResetStage()
{
	// クリアフラグ等を戻す
	_isGameClear = false;
	_goalConfirmOpened = false;
	_goalConfirmResult = ModeGoalConfirm::Result::None;

	// オブジェクトサーバーは以下を全消去
	if(_objectServer)
	{
		_objectServer->ClearObject();
	}

	// --- 再生中のエフェクトを途中でもいいから即時停止 ---
	// _effectBase 内の全エフェクトに対して StopPlaying を呼ぶ（途中停止を許容）
	//for(auto& effectBase : _effectBase)
	//{
	//	if(effectBase)
	//	{
	//		effectBase->StopPlaying();
	//	}
	//}
	//// メンバで保持しているエフェクトハンドルにも念のため停止を要求
	//if(_hensinEffect) _hensinEffect->StopPlaying();
	//if(_walkEffect)  _walkEffect->StopPlaying();
	//if(_findEffect)  _findEffect->StopPlaying();
	//if(_hatenaEffect) _hatenaEffect->StopPlaying();
	//if(_aseEffect)   _aseEffect->StopPlaying();
	//if(_doyaEffect)  _doyaEffect->StopPlaying();
	//if(_nakiEffect)  _nakiEffect->StopPlaying();

	// キャラ/オブジェクト/UI を廃棄
	for(auto& chara : _chara) {
		if(chara) chara->Terminate();
	}
	_chara.clear();

	for(auto& object : _object) {
		if(object) object->Terminate();
	}
	_object.clear();

	for(auto& player_base : _playerBase) {
		if(player_base) player_base->Terminate();
	}
	_playerBase.clear();

	for(auto& ui_base : _uiBase) {
		if(ui_base) ui_base->Terminate();
	}
	_uiBase.clear();

	for(auto& shadow : _charaShadow) {
		if(shadow) shadow->Terminate();
	}
	_charaShadow.clear();

	// --- エフェクトの確実な破棄 ---
	// 個々の EffectBase インスタンスを終了させ、コンテナとメンバ shared_ptr を解放する
	for(auto& effectBase : _effectBase)
	{
		if(effectBase)
		{
			effectBase->Terminate();
		}
	}
	_effectBase.clear();

	// メンバで保持しているエフェクト shared_ptr をリセットして参照を切る
	_hensinEffect.reset();
	_walkEffect.reset();
	_findEffect.reset();
	_hatenaEffect.reset();
	_aseEffect.reset();
	_doyaEffect.reset();
	_nakiEffect.reset();

	// Effekseer に登録されているエフェクトリソースも全削除しておく
	// （再構築時に再ロードさせるため）
	EffekseerManager::GetInstance()->DeleteAllEffect();
	EffekseerManager::GetInstance()->Update();
	EffekseerManager::GetInstance()->StopAllPlayingEffect();

	_enemyBase.clear();
	_enemy.clear();
	_enemyMove.clear();
	_treasure.clear();
	// - グローバルサウンドサーバーも停止（歩行音などグローバルに登録されている音を確実に停止）
	if(gGlobal._soundServer)
	{
		gGlobal._soundServer->StopType(soundserver::SoundItemBase::TYPE::SE);
		gGlobal._soundServer->StopType(soundserver::SoundItemBase::TYPE::VOICE);
		gGlobal._soundServer->StopType(soundserver::SoundItemBase::TYPE::ONESHOT);
		gGlobal._soundServer->StopType(soundserver::SoundItemBase::TYPE::BGM);

		// 個別に保持しているBGMハンドルが再生中なら停止
		if(_bgmInitialize)
		{
			if(_bgmInitialize->IsPlay()) _bgmInitialize->Stop();
		}
		if(_bgmChenge)
		{
			if(_bgmChenge->IsPlay()) _bgmChenge->Stop();
		}

		// 歩行SEなどキー指定で確実に止めたい場合はここで取得して停止
		auto walkSound = gGlobal._soundServer->Get("1");
		if(walkSound && walkSound->IsPlay()) walkSound->Stop();
	}

	// 3Dサウンドも全停止（存在すれば）
	if(_sound3D)
	{
		_sound3D->StopAll();
	}

	// 再構築
	_bShowTanuki = true;
	_showMonoPlayer = false;

	// オブジェクトサーバーの再ロード　
	if(_objectServer == nullptr)
	{
		_objectServer = NEW ObjectServer(this);
	}
	_objectServer->LoadDate("SM_stagebeta");
	_objectServer->ProcessInit();

	// カメラセット
	if(auto* map = _objectServer->GetMap())
	{
		map->SetCamera(_camera);
	}

	ObjectInitialize();	// オブジェクト初期化

	// 各種　initialize
	for(auto& chara : _chara) {
		if(chara) chara->Initialize();
	}
	for(auto& object : _object) {
		if(object) object->Initialize();
	}
	for(auto& player_base : _playerBase) {
		if(player_base) player_base->Initialize();
	}
	for(auto& ui_base : _uiBase) {
		if(ui_base) ui_base->Initialize();
	}
	for(auto& effectBase : _effectBase) {
		if(effectBase) effectBase->Initialize();
	}

	LoadStageData(); // ステージデータ読み込み

	// カメラをプレイヤーがいる位置に合わせる

	if(_camera != nullptr && _playerTanuki)
	{
		// 現在のカメラオフセットを保存（pos - target）
		vec::Vec3 camDelta = vec3::VSub(_camera->_vPos, _camera->_vTarget);

		// ターゲットはタヌキの高さを少し上げた位置にする
		vec::Vec3 target = vec3::VAdd(_playerTanuki->GetPos(), vec3::VGet(0.0f, 60.0f, 0.0f));
		_camera->_vTarget = target;

		// pos をターゲット + オフセットで再計算
		_camera->_vPos = vec3::VAdd(target, camDelta);
	}
	if(_player)       _player->SetCamera(_camera);
	if(_playerTanuki) _playerTanuki->SetCamera(_camera);
	if(_playerMono)   _playerMono->SetCamera(_camera);

	// エフェクトに対象をセット
	if(_treasureEffect)_treasureEffect->SetTreasure(_treasure);
	if(_findEffect) _findEffect->SetEnemy(_enemyBase);
	if(_hatenaEffect) _hatenaEffect->Enemy(_enemyBase);
	if(_walkEffect) _walkEffect->SetPlayerPos(_playerTanuki.get());
	if(_aseEffect)
	{
		_aseEffect->SetEnemy(_enemyBase);
		_aseEffect->SetPlayer(_playerTanuki.get());
	}

	// 宝箱も閉じる（全宝箱）
	for(auto& t : _treasure)
	{
		if(t) t->SetOpen(false);
	}

	// タイマーが動いてたらリセット
	_changeTimeActive = false; // 時間制限を無効化
	_changeTimeLimit = 0.0f;
	_changeBlinkTimer = 0.0f;
	_changeBlinkVisible = true;

	return true;
}