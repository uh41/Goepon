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
	if(!base::Initialize()) { return false; }

	ObjectInitialize();	// オブジェクト初期化

	// オブジェクトサーバー初期化
	_objectServer = NEW ObjectServer(this);
	_objectServer->LoadDate("stage");
	_objectServer->ProcessInit();

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
	if (auto* map = _objectServer->GetMap())
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
	//_bgmInitialize->Play();

	return true;
}

// 終了
bool ModeGame::Terminate()
{
	base::Terminate();

	delete _objectServer;
	_objectServer = nullptr;

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
	if (_enemySensor)
	{
		_enemySensor->Terminate();
		_enemySensor.reset();
	}
	if(_sound3D)	{
		_sound3D->StopAll();
		_sound3D.reset();
	}

	return true;
}

bool ModeGame::LoadStageData()
{
	std::string path = "res/map/";
	std::string jsonFile = "Treasure3.json";
	std::string jsonObjectName = "stage";

	std::ifstream ifs(path + jsonFile);
	nlohmann::json jsonData;
	ifs >> jsonData;
	nlohmann::json stage = jsonData.at(jsonObjectName);

	// customId（文字列）ごとの巡回点リストを保持
	std::unordered_map<std::string, at::vet<vec::Vec3>> patrolGroups;

	// 敵の JSON を一時保存して後で巡回グループを割り当てる
	std::vector<nlohmann::json> enemyObjects;

	for(auto& object : stage)
	{
		const std::string& name = object.at("objectName");

		// ★ S_MarkerR を検出 — customId でグループ化
		if(name == "S_MarkerR")
		{
			vec::Vec3 pos;
			object.at("translate").at("x").get_to(pos.x);
			object.at("translate").at("y").get_to(pos.z); // UE:y -> DXLib:z
			object.at("translate").at("z").get_to(pos.y); // UE:z -> DXLib:y
			pos.z *= -1.0f;

			std::string gid = "";
			if(object.contains("customId"))
			{
				object.at("customId").get_to(gid);
			}
			patrolGroups[gid].push_back(pos);
			continue;
		}

		if(name == "S_MarkerA")
		{
			_playerTanuki->SetJsonDataUE(object);
			continue;
		}

		// 敵は一旦保留（後で customId に対応した巡回点を割り当てる）
		if(name == "S_MarkerB" || name == "S_MarkerRX")
		{
			enemyObjects.push_back(object);
			continue;
		}

	/*	if(name == "Treasure")
		{
			auto _treasure = std::make_shared<Treasure>();
			_treasure->SetJsonDataUE(object);
			continue;
		}*/

		if (name == "Treasure")
		{
			auto treasure = std::make_shared<Treasure>();
			treasure->Initialize();          // モデル読み込み・当たり判定フレーム設定
			treasure->SetJsonDataUE(object); // UE座標/回転を反映
			_treasure.emplace_back(treasure);
			continue;
		}
	}

	// 敵を生成して、customId にマッチする巡回点を割り当てる
	for(auto& object : enemyObjects)
	{
		// センサー類の生成
		auto sensor = std::make_shared<EnemySensor>();
		sensor->Initialize();
		sensor->SetMap(_objectServer->GetMap());

		auto soundSensor = std::make_shared<EnemySoundSensor>();
		soundSensor->Initialize();
		soundSensor->SetMap(_objectServer->GetMap());
		soundSensor->SetSoundSensorArea(300.0f);

		const std::string& name = object.at("objectName");

		// customId を取得（無ければ空文字 "" を使う）
		std::string gid = "";
		if(object.contains("customId"))
		{
			object.at("customId").get_to(gid);
		}

		if(name == "S_MarkerRX")
		{
			auto enemy = std::make_shared<Enemy>();
			enemy->Initialize();
			enemy->SetJsonDataUE(object);
			enemy->SetEnemySensor(sensor);
			enemy->SetEnemySoundSensor(soundSensor);
			soundSensor->SetPos(enemy->GetPos());
			enemy->SetEffect(_hensinEffect);
			_enemyBase.emplace_back(enemy);
			continue;
		}

		if(name == "S_MarkerB")
		{
			auto enemyMove = std::make_shared<EnemyMove>();
			enemyMove->Initialize();
			enemyMove->SetJsonDataUE(object);

			auto sensor = std::make_shared<EnemySensor>();
			sensor->Initialize();
			sensor->SetMap(_objectServer->GetMap());
			//enemy->SetEnemySensor(sensor);
			enemyMove->SetEnemySensor(sensor);

			auto soundSensor = std::make_shared<EnemySoundSensor>();
			soundSensor->Initialize();
			soundSensor->SetMap(_objectServer->GetMap());
			soundSensor->SetSoundSensorArea(300.0f); // 半径300の円形範囲
			soundSensor->SetPos(enemyMove->GetPos());
			enemyMove->SetEnemySensor(sensor);
			enemyMove->SetEnemySoundSensor(soundSensor);
			soundSensor->SetPos(enemyMove->GetPos());
			enemyMove->SetEffect(_hensinEffect);

			// グループに対応する巡回点があれば割り当てる
			auto it = patrolGroups.find(gid);
			if(it != patrolGroups.end() && !it->second.empty())
			{
				enemyMove->SetPatrolPoint(it->second);
				enemyMove->CaptureInitialTransform();
			}

			_enemyBase.emplace_back(enemyMove);
		}

		if (name == "Dog") 
		{
			auto enemyDog = std::make_shared<EnemyDog>();
			enemyDog->Initialize();
			enemyDog->SetJsonDataUE(object);

			auto sensor = std::make_shared<EnemySensor>();
			sensor->Initialize();
			sensor->SetMap(_objectServer->GetMap());
			enemyDog->SetEnemySensor(sensor);

			auto soundSensor = std::make_shared<EnemySoundSensor>();
			soundSensor->Initialize();
			soundSensor->SetMap(_objectServer->GetMap());
			soundSensor->SetSoundSensorArea(900.0f);
			soundSensor->SetPos(enemyDog->GetPos());
			enemyDog->SetEnemySensor(sensor);
			enemyDog->SetEnemySoundSensor(soundSensor);
			enemyDog->SetEffect(_hensinEffect);

			_enemyBase.emplace_back(enemyDog);
		}
	}

	// 互換のため全敵に初期トランスフォームを確実にキャプチャ
	for(auto& enemy : _enemyBase)
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

	// ★クリア画面が消えた後にここが回り始める想定なので、ここで実行するのが安全
	if (_requestResetStage)
	{
		_requestResetStage = false;
		ResetStage();
		return true;
	}

	ModeServer::GetInstance()->SkipProcessUnderLayer();
	ModeServer::GetInstance()->SkipRenderUnderLayer();

	_camera->Process();

	DebugProcess();
	DebugCameraControl();

	//_soundServer->Update();
	AnimationManager::GetInstance()->Update(1.0f);
	// Effekseer 更新
	EffekseerManager::GetInstance()->Update();

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

	PlayerTransform(); // プレイヤー変身処理
	ObjectProcess();   // オブジェクト処理
	
	// 敵との当たり判定処理（生存している敵のみ）
	// 	...
	// 当たり判定の処理をここに書く
	if(_d_use_collision)
	{
		CheckAllDetections();
	}


	// 敵AI（追跡/移動はここで実行される）
	for(auto& enemy : _enemy)
	{
		if(enemy->IsAlive())
		{
			enemy->Process();
		}
	}
	
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
					//ここでゲームオーバー処理へ移行
					ModeServer::GetInstance()->Add(new ModeGameOver(this), 255, "ModeGameOver");

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
	if (CanGoal() && !_isGameClear)
	{
		// いま操作/表示しているプレイヤーで判定
		PlayerBase* goalPlayer = nullptr;

		// ゴール判定を行うプレイヤーを明示的に選択（タヌキ / Mono / 通常）
		if (_bShowTanuki)
		{
			goalPlayer = _playerTanuki.get();
		}
		else if (_showMonoPlayer)
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
			// 正しくタイマーで点滅を制御する
			_changeBlinkTimer += dt;
			if(_changeBlinkTimer >= _changeBlinkInterval)
			{
				_changeBlinkTimer = 0.0f; // タイマーリセット
				_changeBlinkVisible = !_changeBlinkVisible; // 点滅反転
			}
		}
	}

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
	DebugRender(); // デバック描画処理

	if(_d_view_collision)
	{
		//CollisionManager::GetInstance()->SetDebugDraw(true);
	}

	return true;
}

bool ModeGame::UpdateGoalConfirm(PlayerBase* player)
{
	// プレイヤーがいて、未クリア状態で、ゴールに触れているか
	bool hitGoal = (!_isGameClear && player && PlayerToGoalHitCollision(player, _goal.get()));

	// ゴールから離れたら抑制解除（＝次に踏んだらまた確認OK）
	if (!hitGoal)
	{
		_notGoalFlag = false;
	}

	// 抑制中は確認を開かない（No直後に乗りっぱなしでも再表示しない）
	if (_notGoalFlag)
	{
		return false;
	}

	 // 踏んだ瞬間に確認モードを開く
	if(hitGoal && !_goalConfirmOpened)
	{
		_goalConfirmOpened = true;
		_goalConfirmResult = ModeGoalConfirm::Result::None;

		ModeServer::GetInstance()->Add(new ModeGoalConfirm(&_goalConfirmResult), 256, "ModeGoalConfirm");
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
			ModeServer::GetInstance()->Add(new ModeGameClear(this), 255, "ModeGameClear");	
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

	// キャラ/オブジェクト/UI/エフェクトを廃棄
	for(auto& chara       : _chara      ) { if(chara) chara->Terminate();             }
	_chara.clear();
	 
	for(auto& object      : _object     ) { if(object) object->Terminate();           }
	_object.clear();

	for(auto& player_base : _playerBase ) { if(player_base) player_base->Terminate(); }
	_playerBase.clear();

	for(auto& ui_base     : _uiBase     ) { if(ui_base) ui_base->Terminate();         }
	_uiBase.clear();

	for (auto& shadow     : _charaShadow) { if(shadow) shadow->Terminate();           }
	_charaShadow.clear();

	for (auto& effectBase : _effectBase ) { if(effectBase) effectBase->Terminate();   }
	_effectBase.clear();

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
	_bShowTanuki    = true;
	_showMonoPlayer = false;

	// オブジェクトサーバーの再ロード　
	if(_objectServer == nullptr)
	{
		_objectServer = new ObjectServer(this);
	}
	_objectServer->LoadDate("stage");
	_objectServer->ProcessInit();

	// カメラセット
	if (auto* map = _objectServer->GetMap())
	{
		map->SetCamera(_camera);
	}

	ObjectInitialize();	// オブジェクト初期化

	// 各種　initialize
	for(auto& chara       : _chara     ) { if(chara) chara->Initialize();             }
	for(auto& object      : _object    ) { if(object) object->Initialize();           }
	for(auto& player_base : _playerBase) { if(player_base) player_base->Initialize(); }
	for(auto& ui_base     : _uiBase    ) { if(ui_base) ui_base->Initialize();         }
	for(auto& _effectBase : _effectBase) { if(_effectBase) _effectBase->Initialize(); }

	LoadStageData(); // ステージデータ読み込み

	// カメラをプレイヤーがいる位置に合わせる
	if(_player      )       _player->SetCamera(_camera);
	if(_playerTanuki) _playerTanuki->SetCamera(_camera);
	if(_playerMono  )   _playerMono->SetCamera(_camera);

	// エフェクトに対象をセット
	if (_treasureEffect             )_treasureEffect->SetTreasure(_treasure);
	if (_findEffect					) _findEffect->SetEnemy(_enemyBase);
	if (_hatenaEffect				) _hatenaEffect->Enemy(_enemyBase);
	if(_walkEffect					) _walkEffect->SetPlayerPos(_playerTanuki.get());
	if (_aseEffect					)
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