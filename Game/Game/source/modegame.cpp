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
#include "ModeTitle.h"
#include "ModeAffterScenario.h"

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
	/*_requestResetStage = false;*/
	_requestNextStage  = false;

	if(!base::Initialize()) { return false; }

	_originalCamera = nullptr;
	_cinematicCamera = nullptr;
	_camera = nullptr;
	_useCinematicCamera = false;
	_debugZoomActive = false;
	_debugF1KeyPressed = false;

	// ステージマネージャーにステージを登録
	_stageManager.SetStages(gGlobal.GetStageList());

	// 初期ステージIDが設定されていれば、そのIDに対応するステージを現在のステージとして設定する
	if(!_initialStageId.empty())
	{
		// 指定IDが存在すればインデックスを設定する（存在しなければ false を返すが無視可）
		if(!_stageManager.SetCurrentStageId(_initialStageId))
		{
			// デバッグ出力：指定IDが見つからなかった

			DrawFormatString(10, 20, GetColor(255, 0, 0), "Warning: initial stage id '%s' not found", _initialStageId.c_str());
		}
	}

	ObjectInitialize();	// オブジェクト初期化

	_objectServer = ApplicationMain::GetInstance()->GetObjectServer();

	// オブジェクトサーバーでマップデータを読み込み
	_objectServer->LoadDate(_stageManager.GetCurrentStageId());

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

	// ★★★ カメラ初期化をここで行う ★★★
	DebugInitialize();// デバック初期化
	ShadowInitialize();// シャドウ生成
	CameraInfoInitialize();// カメラ情報初期化 ← ここで_cameraが作成される


	//// カメラをプレイヤー位置に合わせる（JSONでプレイヤー位置を読み込んだ直後に適用）
	//if(_camera != nullptr)
	//{
	//	// カメラの現在のオフセット（pos - target）を保存しておき、プレイヤーに合わせて再設定する
	//	vec::Vec3 camDelta = vec3::VSub(_camera->GetPos(), _camera->GetTarget());

	//	// 初期表示プレイヤー（タヌキ／人間）に合わせる
	//	PlayerBase* startPlayer = nullptr;
	//	if(_bShowTanuki)
	//	{
	//		startPlayer = _playerTanuki.get();
	//	}
	//	else
	//	{
	//		startPlayer = _player.get();
	//	}

	//	if(startPlayer != nullptr)
	//	{
	//		// ターゲットはプレイヤーの高さを少し上げて注視する（元のカメラ設定に合わせる）
	//		vec::Vec3 target = vec3::VAdd(startPlayer->GetPos(), vec3::VGet(0.0f, 100.0f, 0.0f));
	//		_camera->SetTarget(target);
	//		_camera->SetPos(vec3::VAdd(target, camDelta));
	//	}
	//}

	// カメラセット
	if(auto* map = _objectServer->GetMap())
	{
		map->SetCamera(_camera);
	}
	
	// 設定
	//_cinematicCamera->_vPos = vec::Vec3(0.0f, 100.0f, 0.0f);      // 例：初期位置
	//_cinematicCamera->_vTarget = vec::Vec3(0.0f, 0.0f, 0.0f);     // 例：注視点
	//_cinematicCamera->_fClipNear = 1.0f;
	//_cinematicCamera->_fClipFar = 1000.0f;
	

	_bShowTanuki = true;

	_player->SetCamera(_camera);
	_playerTanuki->SetCamera(_camera);
	_playerMono->SetCamera(_camera);
	_treasureEffect->SetTreasure(_treasureBase);
	_walkEffect->SetPlayerPos(_playerTanuki.get());
	_findEffect->SetEnemy(_enemyBase);
	_aseEffect->SetPlayer(_playerTanuki.get());
	_hatenaEffect->Enemy(_enemyBase);
	_aseEffect->SetEnemy(_enemyBase);

	_bResolveOnY = false;
	_bLandedOnUp = false;
	_bCameraControlMode = false;
	_hasSavedCameraState = false;

	_isChengeBgm = false;

	_soundFinish = gGlobal._soundServer->Get("3");

	// BGM再生
	_bgmInitialize = gGlobal._soundServer->Get("bgminitialize");
	_bgmChenge = gGlobal._soundServer->Get("bgmChenge");
	_bgmInitialize->Play();

	_isLoadComplete = true; // ロード完了フラグを立てる

	// イントロ演出の初期化を追加
	_isIntroActive = false;
	_introButtonPressed = false;
	_introTimer = 0.0f;

	//// カメラが正常に初期化された後にイントロ演出を開始 ★★★
	//if (_camera != nullptr)
	//{
	//	StartIntroSequence();
	//}
	return true;	
}

// 終了
bool ModeGame::Terminate()
{
	base::Terminate();

	// 演出カメラの安全な削除
	if(_cinematicCamera)
	{
		_cinematicCamera->StopAll();

		// _cameraが_cinematicCamera.get()を指している場合、元のカメラに戻す
		if(_camera == _cinematicCamera.get())
		{
			_camera = _originalCamera; // nullptrではなく元のカメラを設定
		}

		_cinematicCamera.reset();
	}

	_useCinematicCamera = false;
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

	for(auto& tutorial : _tutorial)
	{
		if(tutorial)
		{
			tutorial->Terminate();
		}
	}
	_tutorial.clear();

	// カメラの削除を最後に行う
	if(_camera && _camera != _originalCamera)
	{
		// _cameraが_originalCameraと同じでない場合のみ削除
		delete _camera;
	}
	_camera = nullptr;

	if(_originalCamera)
	{
		delete _originalCamera;
		_originalCamera = nullptr;
	}

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

	if(_soundFinish)
	{
		_soundFinish->Stop();
		_soundFinish = nullptr;
	}

	return true;
}

// 敵センサーの生成を共通化するヘルパー関数
at::spc<EnemySensor> ModeGame::CreateEnemySensor(float soundArea, MapBase* map)
{
	auto sensor = std::make_shared<EnemySensor>();
	sensor->Initialize();
	sensor->SetMap(map);
	
	return sensor;	
}

void ModeGame::CreateEnemy
(
	const nlohmann::json& object,
	const enemygroup& patrolGroup,
	const enemygroup& dogMovementArea,
	uint32_t& nextEnemyId,
	MapBase* map
)
{
	const std::string& name = object.at("objectName");

	std::string gid;
	if(object.contains("customId"))
	{
		object.at("customId").get_to(gid);
	}

	// 動かない敵
	if(name == "S_MarkerRX")
	{
		auto sensor = CreateEnemySensor(300.0f, map);
		sensor->SetDetectionSector(400.0f, 120.0f);

		auto enemy = std::make_shared<Enemy>();
		enemy->Initialize();
		enemy->SetJsonDataUE(object);
		enemy->SetEnemySensor(sensor);
		enemy->SetEffect(_hensinEffect);
		enemy->SetEffect(_shirimochiEffect);
		enemy->SetStunEffect(_stunEffect);
		enemy->SetEnemyId(nextEnemyId++);
		enemy->SetDirSequenceFromJson(object);

		_enemyBase.emplace_back(std::move(enemy));
		return;
	}

	// 動く敵
	if(name == "S_MarkerB")
	{
		auto sensor = CreateEnemySensor(300.0f, map);
		sensor->SetDetectionSector(400.0f, 120.0f);

		auto enemyMove = std::make_shared<EnemyMove>();
		enemyMove->Initialize();
		enemyMove->SetJsonDataUE(object);
		enemyMove->SetEnemySensor(sensor);
		enemyMove->SetEffect(_hensinEffect);
		enemyMove->SetEffect(_shirimochiEffect);
		enemyMove->SetStunEffect(_stunEffect);
		enemyMove->SetEnemyId(nextEnemyId++);
		enemyMove->SetDirSequenceFromJson(object);

		// 巡回グループの割り当て
		const ApplicationGlobal::StageData* stageData = gGlobal.GetStageData(_stageManager.GetCurrentStageId());
		if(stageData)
		{
			auto itInfo = stageData->patrolPointInfo.find(gid);
			if(itInfo != stageData->patrolPointInfo.end() && !itInfo->second.empty())
			{
				enemyMove->SetPatrolPointInfo(itInfo->second);
				enemyMove->CaptureInitialTransform();
				_enemyBase.emplace_back(std::move(enemyMove));
				return;
			}
		}

		// フォールバック（従来の座標のみ）
		auto it = patrolGroup.find(gid);
		if(it != patrolGroup.end() && !it->second.empty())
		{
			enemyMove->SetPatrolPoint(it->second);
			enemyMove->CaptureInitialTransform();
		}

		_enemyBase.emplace_back(std::move(enemyMove));
		return;
	}


	// 犬
	if(name == "Dog")
	{
		auto sensor = CreateEnemySensor(900.0f, map);
		sensor->SetDetectionSector(400.0f, 120.0f);

		auto dog = std::make_shared<EnemyDog>();
		dog->Initialize();
		dog->SetJsonDataUE(object);
		dog->SetEnemySensor(sensor);
		dog->SetEffect(_hensinEffect);
		dog->SetEnemyId(nextEnemyId++);

		// 犬の移動範囲の割り当て
		auto it = dogMovementArea.find(gid);
		if(it != dogMovementArea.end() && !it->second.empty())
		{
			dog->SetMovementArea(it->second);// 犬の移動範囲を設定
		}

		_enemyBase.emplace_back(std::move(dog));
		return;
	}
}

bool ModeGame::LoadStageData()
{
	const ApplicationGlobal::StageData* stageData = gGlobal.GetStageData(_stageManager.GetCurrentStageId());
	if(stageData == nullptr)
	{
		return false;
	}

	auto& patrolGroup = stageData->patrolGroup;
	const auto& objectList = stageData->object;

	// 敵の JSON を一時保存して後で巡回グループを割り当てる
	std::vector<nlohmann::json> enemyObjects;

	static const at::ust<std::string> skipObject = { "S_MarkerR" };
	static const at::ust<std::string> enemyObject = { "S_MarkerB", "S_MarkerRX", "Dog" };

	at::vet<nlohmann::json> enemyJsonList; // 敵オブジェクトの JSON を一時保存するリスト

	// 犬用の移動範囲を保持
	std::unordered_map<std::string, std::vector<vec::Vec3>> dogMovementAreas;

	uint32_t nextEnemyId = 1; // 敵IDのカウンタ（1からスタート）

	// **修正: 二重ループを削除し、1回のループで全オブジェクトを処理**
	for(auto&& objData : stageData->object)
	{
		const std::string& name = objData.objectName;

		// スキップ対象のオブジェクトは処理しない
		if(skipObject.find(name) != skipObject.end())
		{
			continue; // スキップ対象のオブジェクトは処理しない
		}

		const nlohmann::json& object = objData.json;

		// ★ S_MarkerDGR を検出 — 犬の移動範囲
		if (name == "S_MarkerDGR")
		{
			vec::Vec3 pos;
			object.at("translate").at("x").get_to(pos.x);
			object.at("translate").at("y").get_to(pos.z);
			object.at("translate").at("z").get_to(pos.y);
			pos.z *= -1.0f;

			std::string gid = "";
			if (object.contains("customId"))
			{
				object.at("customId").get_to(gid);
			}
			dogMovementAreas[gid].push_back(pos);
			continue;
		}

		//プレイヤー開始位置の設定
		if(name == "S_MarkerA")
		{
			_playerTanuki->SetJsonDataUE(object);
			continue;
		}

		// 敵オブジェクト
		if(enemyObject.count(name))
		{
			enemyJsonList.push_back(object);
			continue;
		}

		if(name == "Goal")
		{
			_goal->SetJsonDataUE(object);
			continue;
		}

		if(name == "Treasure")
		{
			//auto treasure = std::make_shared<Treasure>();
			auto treasure = std::make_shared<TreasureRapidFire>();
			treasure->Initialize();
			treasure->SetJsonDataUE(object);
			_treasureBase.emplace_back(treasure);
			_treasureRapidFire.emplace_back(treasure); 
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

		if(name == "S_Marker_Event")
		{
			auto tutorial = std::make_shared<Tutorial>();
			tutorial->Initialize();
			tutorial->SetJsonDataUE(object);

			std::string customId;
			if(object.contains("customId"))
			{
				object.at("customId").get_to(customId);

				if(!customId.empty())
				{
					tutorial->SetEventId(std::stoi(customId));
				}
			}
			else
			{
				tutorial->SetEventId(0); // デフォルトのイベントIDを設定
			}

			_tutorial.emplace_back(tutorial);
			continue;
		}
	}

	if(_counterUi)
	{
		_counterUi->SetTreasureCount(static_cast<int>(_treasure.size()));
	}
	

	// **最適化: センサー生成を共通化するヘルパー関数**
	auto* map = _objectServer->GetMap();
	for(auto&& object : enemyJsonList)
	{
		CreateEnemy(object, patrolGroup, dogMovementAreas, nextEnemyId, map);
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
	vec::Vec3 newPos = vec3::VAdd(_camera->GetPos(), playermove);
	vec::Vec3 newTarget = vec3::VAdd(_camera->GetTarget(), playermove);
	_camera->SetPos(newPos);
	_camera->SetTarget(newTarget);
	return true;
}

// 計算処理
bool ModeGame::Process()
{
	base::Process();

	//// イントロ演出中の処理
	//if (_isIntroActive)
	//{
	//	ProcessIntroSequence();

	//	// イントロ中でもメインゲームと同様の処理を実行 ★★★
	//	if (_isIntroActive)
	//	{
	//		// カメラ処理
	//		if (_camera)
	//		{
	//			_camera->Process();
	//		}

	//		// アニメーション処理（プレイヤーのアニメーション用）
	//		AnimationManager::GetInstance()->Update(1.0f);

	//		// オブジェクトサーバー処理（マップ描画のため）
	//		_objectServer->ProcessInit(); // 追加・削除予約の確定
	//		_objectServer->Process();

	//		// オブジェクト処理（最小限）
	//		ObjectProcess();

	//		// プレイヤー変身処理（アニメーション継続のため）
	//		PlayerTransform();

	//		// エフェクト処理（必要に応じて）
	//		for (auto& effectBase : _effectBase)
	//		{
	//			effectBase->Process();
	//		}

	//		return true;
	//	}
	//}

	ModeServer::GetInstance()->SkipProcessUnderLayer();
	ModeServer::GetInstance()->SkipRenderUnderLayer();

	/*if(_requestResetStage)
	{
		_requestResetStage = false;
		ResetStage();
		return true;
	}*/

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

	/*if(_bShowTanuki)
	{
		EscapeCollision(_playerTanuki.get(), _objectServer->GetMap());
		const bool hitTreasure = CharaToTreasureHitCollision(_playerTanuki.get(), _treasureBase);
		CharaToTreasureOpenCollision(_playerTanuki.get(), _treasureBase);
		PlayerCameraInfo(_playerTanuki.get());
	}
	else if(_showMonoPlayer)
	{
		EscapeCollision(_playerMono.get(), _objectServer->GetMap());
		const bool hitTreasure = CharaToTreasureHitCollision(_playerMono.get(), _treasureBase);
		CharaToTreasureOpenCollision(_playerMono.get(), _treasureBase);
		PlayerCameraInfo(_playerMono.get());
	}
	else
	{
		EscapeCollision(_player.get(), _objectServer->GetMap());
		const bool hitTreasure = CharaToTreasureHitCollision(_player.get(), _treasureBase);
		CharaToTreasureOpenCollision(_player.get(), _treasureBase);
		PlayerCameraInfo(_player.get());
	}*/

	if (_bShowTanuki)
	{
		EscapeCollision(_playerTanuki.get(), _objectServer->GetMap());
		const bool hitTreasure = CharaToTreasureHitCollision(_playerTanuki.get(), _treasureBase);
		// 長押し型から連打型に変更
		CharaToTreasureRapidFireCollision(_playerTanuki.get(), _treasureRapidFire);
		PlayerCameraInfo(_playerTanuki.get());
	}
	else if (_showMonoPlayer)
	{
		EscapeCollision(_playerMono.get(), _objectServer->GetMap());
		const bool hitTreasure = CharaToTreasureHitCollision(_playerMono.get(), _treasureBase);
		CharaToTreasureRapidFireCollision(_playerMono.get(), _treasureRapidFire);
		PlayerCameraInfo(_playerMono.get());
	}
	else
	{
		EscapeCollision(_player.get(), _objectServer->GetMap());
		const bool hitTreasure = CharaToTreasureHitCollision(_player.get(), _treasureBase);
		CharaToTreasureRapidFireCollision(_player.get(), _treasureRapidFire);
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

	PlayerToTutorialCollision(playerBase, _tutorial);

	bool hasCollision = false;

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

					_hasRenderOnce = false;

					//ここでゲームオーバー処理へ移行
					ModeServer::GetInstance()->Add(NEW ModeGameOver(this), 255, "ModeGameOver");

					break;
				}
				// 実際の押し出し（カプセル）
				// 敵に接触したときに実際に行う処理はここで記入
				hasCollision = true;
			}
		}
	}

	// プレイヤーと敵の接触があった場合、敵の音を発生させる
	if (hasCollision && playerBase&& _bShowTanuki)
	{
		EnemySoundManager::GetInstance()->EmitSound(
			playerBase->GetPos(),  // 位置
			1,					// 音の大きさレベル（1-3で調整）
			400.0f,				// 音波の最大半径
			10.0f				// 音波の速度
		);
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
				vec::Vec3 camDelta = vec3::VSub(_camera->GetPos(), _camera->GetTarget());
				vec::Vec3 target = vec3::VAdd(_playerTanuki->GetPos(), vec3::VGet(0.0f, 60.0f, 0.0f));
				_camera->SetTarget(target);
				_camera->SetPos(vec3::VAdd(target, camDelta));
			}

			// エフェクト再設定
			_hensinEffect->PlayEffect(_playerTanuki->GetPos());
			_walkEffect->SetPlayerPos(_playerTanuki.get());
			_aseEffect->SetPlayer(_playerTanuki.get());

			if(_soundFinish && !_soundFinish->IsPlay())
			{
				_soundFinish->Play();
			}

			// 周囲の敵に対する音波／エフェクト
			for(auto& enemy : _enemyBase)
			{
				if(enemy && enemy->IsAlive())
				{
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
	// **カメラの有効性をチェック**
	if(!_camera)
	{
		return false; // カメラが無効な場合は描画をスキップ
	}

	base::Render();

	// カメラ設定更新
	SetCameraPositionAndTarget_UpVecY(DxlibConverter::VecToDxLib(_camera->GetPos()), DxlibConverter::VecToDxLib(_camera->GetTarget()));
	SetCameraNearFar(_camera->GetClipNear(), _camera->GetClipFar());
	float fov_deg = 30.0f;
	float fov_rad = DEG2RAD(fov_deg);
	SetupCamera_Perspective(fov_rad);

	EffekseerManager::GetInstance()->Render();

	// オブジェクトサーバーの描画
	if(_objectServer)
	{
		_objectServer->Render();
	}

	ObjectRender();// オブジェクト描画処理
	DebugRender(); // デバック描画処理

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

	bool noOverlayAbove =
		(ModeServer::GetInstance()->Get("gameover") == nullptr) &&
		(ModeServer::GetInstance()->Get("gameoverload") == nullptr);

	if(noOverlayAbove)
	{
		// opscenario など最初のロードで即時描画済み扱いにしたい場合はここで true にする
		_hasRenderOnce = true;
		_isLoadComplete = true;
	}

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

bool ModeGame::DebugCinematicCameraControl()
{
	if(!_cinematicCamera)
	{
		return false; // 演出カメラが存在しない場合は処理しない
	}

	if(!_originalCamera)
	{
		return false;
	}

	if(!_camera)
	{
		return false; // 現在のカメラが存在しない場合は処理しない
	}

	if(CheckHitKey(KEY_INPUT_F1))
	{
		if(!_debugF1KeyPressed)
		{
			_debugF1KeyPressed = true;
			if(!_debugZoomActive)
			{
				_debugZoomActive = true;
				// 演出カメラに現在のカメラ位置と注目点をコピーして切り替え
				if(_cinematicCamera && _camera)
				{
					// 演出カメラに現在のカメラ位置と注目点をコピーして切り替え
					_cinematicCamera->SetPos(_camera->GetPos());
					_cinematicCamera->SetTarget(_camera->GetTarget());
					_cinematicCamera->SetClipNear(_camera->GetClipNear());
					_cinematicCamera->SetClipFar(_camera->GetClipFar());

					_useCinematicCamera = true;
					_camera = _cinematicCamera.get();
				}
				else
				{
					// カメラが無効な場合は処理をスキップ
					return false;
				}

				// 現在のプレイヤー位置を取得してズーム演出
				PlayerBase* targetPlayer = nullptr;
				if(_bShowTanuki && _playerTanuki && _playerTanuki->IsAlive())
				{
					targetPlayer = _playerTanuki.get();
				}
				else if(_showMonoPlayer && _playerMono && _playerMono->IsAlive())
				{
					targetPlayer = _playerMono.get();
				}
				else if(_player && _player->IsAlive())
				{
					targetPlayer = _player.get();
				}
				if(targetPlayer && _cinematicCamera)
				{
					vec::Vec3 target = targetPlayer->GetPos();
					// 現在位置のカメラの位置からプレイヤーの位置への距離を計算
					vec::Vec3 currentPos = _cinematicCamera->GetPos();
					float currentDist = vec3::VSize(vec3::VSub(currentPos, target)); 
					float endDist = currentDist * 0.25f; // 最終的な距離（半分にする例）
					if(endDist < 50.0f) endDist = 50.0f; // 最小距離を設定（近すぎないように）

					// ズーム演出：現在距離から近距離へ
					_cinematicCamera->StartZoom(target, 0.5f, currentDist, endDist); // ターゲット位置、ズーム倍率、ズーム距離、ズーム時間
				}
			}
			else
			{

				// ズーム解除
				_debugZoomActive = false;
				_useCinematicCamera = false;

				if(_cinematicCamera)
				{
					_cinematicCamera->StopAll();
				}

				// 元のカメラに戻す前に有効性をチェック
				if(_originalCamera)
				{
					_camera = _originalCamera;
				}
			}
		}
	}
	else
	{
		_debugF1KeyPressed = false;
	}
	return true;
}

bool ModeGame::TreasureOpeningCameraControl()
{
	if(_isOpeningTreasure)
	{
		return false; // またはすでに開いている場合は処理しない
	}

	// シネマティックカメラが初期化されていない場合は作成
	if(!_cinematicCamera)
	{
		_cinematicCamera = std::make_unique<CinematicCamera>();
		if(!_cinematicCamera->Initialize())
		{
			_cinematicCamera.reset(); 
			return false;
		}
	}

	// 元のカメラを保存
	if(!_useCinematicCamera)
	{
		if(_cinematicCamera && _camera)
		{
			// 演出カメラに現在のカメラ位置と注目点をコピーして切り替え
			_cinematicCamera->SetPos(_camera->GetPos());
			_cinematicCamera->SetTarget(_camera->GetTarget());
			_cinematicCamera->SetClipNear(_camera->GetClipNear());
			_cinematicCamera->SetClipFar(_camera->GetClipFar());

			_originalCamera = _camera;
			_useCinematicCamera = true;
			_camera = _cinematicCamera.get();
		}
		else
		{
			// カメラがない場合は処理をスキップ
			return false;
		}

		// 現在のプレイヤー位置を取得
		PlayerBase* targetPlayer = nullptr;
		if(_bShowTanuki && _playerTanuki && _playerTanuki->IsAlive())
		{
			targetPlayer = _playerTanuki.get();
		}
		else if(_showMonoPlayer && _playerMono && _playerMono->IsAlive())
		{
			targetPlayer = _playerMono.get();
		}
		else if(_player && _player->IsAlive())
		{
			targetPlayer = _player.get();
		}

		if(targetPlayer && _cinematicCamera)
		{
			vec::Vec3 target = targetPlayer->GetPos();
			vec::Vec3 currentPos = _cinematicCamera->GetPos();
			float currentDist = vec3::VSize(vec3::VSub(currentPos, target));
			float endDist = currentDist * 0.5f; // 最終的な距離（半分にする例）
			if(endDist < 50.0f) endDist = 50.0f; // 最小距離を設定（近すぎないように）

			// ズーム演出：現在距離から近距離へ
			_cinematicCamera->StartZoom(target, 0.5f, currentDist, endDist);
		}
	}
	return true;
}

bool ModeGame::EndCinematicCamera()
{
	if(!_useCinematicCamera || !_cinematicCamera)
	{
		return false;
	}

	// 元のカメラに戻す
	if(_originalCamera)
	{
		_camera = _originalCamera;
		_originalCamera = nullptr;
	}

	_useCinematicCamera = false;

	//　シネマティックカメラをリセット
	_cinematicCamera->StopAll();
	return true;
}

// 初期ステージIDを設定
void ModeGame::SetInitialStageId(const std::string& stageId)
{
	_initialStageId = stageId;
}

bool ModeGame::StartIntroSequence()
{
	// カメラが初期化されていない場合は失敗
	if (!_camera)
	{
		return false;
	}

	// 演出カメラが初期化されていない場合は作成（ObjectInitialize()で既に作成済みの場合はスキップ）
	if (!_cinematicCamera)
	{
		_cinematicCamera = std::make_unique<CinematicCamera>();
		if (!_cinematicCamera->Initialize())
		{
			_cinematicCamera.reset();
			return false;
		}
	}

	// 元のカメラを保持して演出カメラに切り替え
	if (!_useCinematicCamera && _camera)
	{
		_originalCamera = _camera;
		_useCinematicCamera = true;
		_camera = _cinematicCamera.get();

		// プレイヤーの情報を取得
		PlayerBase* targetPlayer = nullptr;
		if (_bShowTanuki && _playerTanuki && _playerTanuki->IsAlive())
		{
			targetPlayer = _playerTanuki.get();
		}
		else if (_player && _player->IsAlive())
		{
			targetPlayer = _player.get();
		}

		if (targetPlayer)
		{
			vec::Vec3 playerPos = targetPlayer->GetPos();
			vec::Vec3 playerDir = targetPlayer->GetDir();

			// プレイヤーの向きが正常でない場合はデフォルト向きを使用
			if (vec3::VSize(playerDir) < 0.001f)
			{
				playerDir = vec3::VGet(0.0f, 0.0f, 1.0f); // デフォルト向き（Z軸正方向）
			}

			// プレイヤーの少し上を注視点に設定
			vec::Vec3 faceTarget = vec3::VAdd(playerPos, vec3::VGet(0.0f, 80.0f, 0.0f));

			// プレイヤーの前方向に少し離れた位置にカメラを配置
			vec::Vec3 cameraOffset = vec3::VScale(playerDir, 150.0f); // 150単位前方に変更
			vec::Vec3 cameraPos = vec3::VAdd(faceTarget, cameraOffset);

			// カメラの設定を適用
			_cinematicCamera->SetPos(cameraPos);
			_cinematicCamera->SetTarget(faceTarget);
			_cinematicCamera->SetClipNear(1.0f);
			_cinematicCamera->SetClipFar(10000.0f);

			// デバッグ出力でカメラ設定を確認
			DrawFormatString(10, 100, GetColor(0, 255, 0),
				"Intro Camera Set - Pos: (%.1f, %.1f, %.1f) Target: (%.1f, %.1f, %.1f)",
				cameraPos.x, cameraPos.y, cameraPos.z, faceTarget.x, faceTarget.y, faceTarget.z);
		}
		else
		{
			// プレイヤーが見つからない場合はデフォルトカメラ設定
			// 元のカメラの設定をコピーして少し調整
			vec::Vec3 originalPos = _originalCamera->GetPos();
			vec::Vec3 originalTarget = _originalCamera->GetTarget();

			// より近い位置に設定
			vec::Vec3 introPos = vec3::VGet(0.0f, 100.0f, -200.0f);
			vec::Vec3 introTarget = vec3::VGet(0.0f, 50.0f, 0.0f);

			_cinematicCamera->SetPos(introPos);
			_cinematicCamera->SetTarget(introTarget);
			_cinematicCamera->SetClipNear(1.0f);
			_cinematicCamera->SetClipFar(10000.0f);

			// デバッグ出力
			DrawFormatString(10, 120, GetColor(255, 255, 0),
				"Intro Camera Default - Pos: (%.1f, %.1f, %.1f) Target: (%.1f, %.1f, %.1f)",
				introPos.x, introPos.y, introPos.z, introTarget.x, introTarget.y, introTarget.z);
		}
	}

	// イントロ演出を開始
	_isIntroActive = true;
	_introButtonPressed = false;
	_introTimer = 0.0f;

	return true;
}

// ProcessIntroSequence()関数を追加
bool ModeGame::ProcessIntroSequence()
{
	// イントロ演出がアクティブでない場合は処理しない
	if (!_isIntroActive)
	{
		return false;
	}

	// 経過時間を更新
	float deltaTime = 1.0f / 60.0f; // 60FPS想定
	_introTimer += deltaTime;

	int trg = ApplicationBase::GetInstance()->GetTrg(); // 入力状態を取得

	// ボタン入力をチェック（スペースキー、エンターキー、またはゲームパッドのAボタン）
	if (!_introButtonPressed)
	{
		if(trg & PAD_INPUT_1)
		{
			_introButtonPressed = true;
		}
	}

	// ボタンが押されたか、時間が経過したらイントロ終了
	if (_introButtonPressed || _introTimer >= INTRO_DURATION)
	{
		EndIntroSequence();
		return true;
	}

	// イントロ中はプレイヤーの操作を無効化
	// （必要に応じてプレイヤーの入力を無視する処理をここに追加）

	return true;
}

// EndIntroSequence()関数を追加
bool ModeGame::EndIntroSequence()
{
	if (!_isIntroActive)
	{
		return false;
	}

	// イントロ演出を終了
	_isIntroActive = false;
	_introButtonPressed = false;
	_introTimer = 0.0f;

	// 元のカメラに戻す
	if (_useCinematicCamera && _originalCamera)
	{
		_camera = _originalCamera;
		_useCinematicCamera = false;

		if (_cinematicCamera)
		{
			_cinematicCamera->StopAll();
		}

		_originalCamera = nullptr; // リセット
	}

	return true;
}