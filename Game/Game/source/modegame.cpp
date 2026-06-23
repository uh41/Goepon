/*********************************************************************/
// * \file   modegame.cpp
// * \brief  モードゲームクラス
// *
// * \author 鈴木裕稀
/*********************************************************************/

#include "modegame.h"
#include "applicationmain.h"
#include "modeeffekseer.h"
#include "ModeGameClear.h"
#include "applicationglobal.h"
#include "ModeGameOver.h"
#include "ModeTitle.h"
#include "ModeAffterScenario.h"
#include "savemanager.h"
#include "effectmanager.h"
#include "playerform.h"

#ifdef _DEBUG
#include <crtdbg.h>
#define NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
#else
#define NEW new
#endif

// 初期化
bool ModeGame::Initialize()
{
	_hasRenderOnce	  = false; // 初回レンダリング前のフラグをリセット
	_requestNextStage = false; // 次のステージへの遷移要求フラグをリセット

	if(!base::Initialize()) { return false; }

	// EffekseerManager の初期化
	if(EffekseerManager::GetInstance() && !_effekseerLaunched)
	{
		// EffekseerManager の Initialize() は存在する想定
		EffekseerManager::GetInstance()->Initialize();
		_effekseerLaunched = true;
	}

	// カメラ変数の初期化
	_savedCamera        = nullptr;
	_cinematicCamera    = nullptr;
	_camera             = nullptr;
	_useCinematicCamera = false;
	_debugZoomActive    = false;
	_debugF1KeyPressed  = false;
	_debugF2KeyPressed  = false;
	_debugShakeActive   = false;

	_tanukiAttackCount = 0;

	// ステージマネージャーにステージを登録
	_stageManager.SetStages(gGlobal.GetStageList());

	// 初期ステージIDが設定されていれば、そのIDに対応するステージを現在のステージとして設定する
	if(!_initialStageId.empty())
	{
		// 指定IDが存在すればインデックスを設定する（存在しなければ false を返すが無視可）
		if(!_stageManager.SetCurrentStageId(_initialStageId))
		{
			// デバッグ出力：指定IDが見つからなかった
			//DrawFormatString(10, 20, GetColor(255, 0, 0), "Warning: initial stage id '%s' not found", _initialStageId.c_str());
		}
	}

	// オブジェクト初期化
	ObjectInitialize();

	// オブジェクトサーバーの初期化（applicationmain から取得）
	_objectServer = ApplicationMain::GetInstance()->GetObjectServer();

	gGlobal.EnsureStageDataLoad(_stageManager.GetCurrentStageId());

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

	// 宝箱のベクターを宝箱ベースから作成
	_treasure.clear();
	for(auto& tb : _treasureBase)
	{
		if(!tb) continue;
		if(auto t = std::dynamic_pointer_cast<Treasure>(tb))
		{
			_treasure.emplace_back(t);
		}
	}

	// 初期カウント: 長押し型 + 連打型 の両方をカウントする
	int totalTreasureCount = 0;
	for(auto& tb : _treasureBase)
	{
		if(!tb) continue;
		// 表示中かつ未開放の宝箱のみをクリア条件に含める
		if(tb->IsVisible() && !tb->IsOpen())
		{
			++totalTreasureCount;
		}
	}
	_treasureTakenCount = 0;
	_treasureRequiredCount = totalTreasureCount;

	// カウンタ/UI に正しい数を反映（全宝箱数を表示）
	if(_counterUi)
	{
		_counterUi->SetTreasureCount(_treasureRequiredCount);
	}

	if(_treasureUi)
	{
		_treasureUi->SetTreasureList(_treasureBase);
	}

	// ゴールは「必要個数が0なら有効化」
	if(_goal)
	{
		_goal->SetCollisionEnabled(_treasureRequiredCount == 0);
	}

	// ゴール初期化
	_isGameClear = false;

	// カメラ初期化をここで行う
	DebugInitialize();// デバック初期化
	CameraInfoInitialize();// カメラ情報初期化 ← ここで_cameraが作成される

	//  カメラが正常に作成されたことを確認 
	if(_camera == nullptr)
	{
		// エラーハンドリング：カメラが作成されていない場合
		return false;
	}

	// カメラをプレイヤー位置に合わせる（JSONでプレイヤー位置を読み込んだ直後に適用）
	if (_camera != nullptr)
	{
		// カメラの現在のオフセット（pos - target）を保存しておき、プレイヤーに合わせて再設定する
		vec::Vec3 camDelta = vec3::VSub(_camera->GetPos(), _camera->GetTarget());

		// 初期表示プレイヤー（タヌキ）に合わせる
		PlayerBase* startPlayer = PlayerFactory::GetTanukiPlayer();

		if(startPlayer != nullptr)
		{
			// ターゲットはプレイヤーの高さを少し上げて注視する
			vec::Vec3 target = vec3::VAdd(startPlayer->GetPos(), vec3::VGet(0.0f, 60.0f, 0.0f));
			_camera->SetTarget(target);
			_camera->SetPos(vec3::VAdd(target, camDelta));
		}
	}

	// カメラセット
	if(auto* map = _objectServer->GetMap())
	{
		map->SetCamera(_camera);
	}
	
	PlayerManager::GetInstance()->SetInitialPlayerState(PlayerManager::PlayerState::TANUKI);

	// プレイヤーとエフェクトにカメラをセット
	PlayerFactory::GetTanukiPlayer()->SetCamera(_camera);
	PlayerFactory::GetHumanPlayer()->SetCamera(_camera);
	PlayerFactory::GetMonoPlayer()->SetCamera(_camera);

	EffectManager::GetTreasureEffect()->SetCamera(_camera);
	EffectManager::GetSavePointEffect()->SetSavePoint(_savePoint);
	EffectManager::GetWalkEffect()->SetPlayerPos(PlayerFactory::GetTanukiPlayer());

	// エフェクト初期設定
	EffectManager::SetInitialPlayer(PlayerFactory::GetTanukiPlayer());
	EffectManager::SetGoal(_goal);
	EffectManager::SetTreasure(_treasureBase);

	_bResolveOnY = false;
	_bLandedOnUp = false;
	_bCameraControlMode = false;
	_hasSavedCameraState = false;

	_isChengeBgm = false;
	_soundFinish = gGlobal._soundServer->Get("3");

	// BGM再生
	_bgmInitialize = gGlobal._soundServer->Get("bgminitialize");
	_bgmChenge = gGlobal._soundServer->Get("bgmChenge");
	if(_bgmChenge)
	{
		_bgmChenge->SetVolume(0); // 再生時は無音にする
		_bgmChenge->Play();       // StreamLoad とハンドル作成を行う
		_bgmChenge->Stop();       // 再生は止めてハンドルを保持する
	}

	_bgmInitialize->Play();

	SavePlayer(nullptr); // プレイヤーの状態をセーブする（必要に応じて引数でセーブスロットを指定できるようにする）

	_isLoadComplete = true; // ロード完了フラグを立てる

	// イントロ演出の初期化
	_isIntroActive = false;
	_introButtonPressed = false;
	_introTimer = 0.0f;

	// カメラが正常に初期化された後にイントロ演出を開始 ★★★
	StartIntroSequence();
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
			_camera = _savedCamera; // nullptrではなく元のカメラを設定
		}

		_cinematicCamera.reset();
	}

	_useCinematicCamera = false;

	// キャラ
	for(auto& chara : _chara)
	{
		if(chara) chara->Terminate();
	}
	_chara.clear();

	for(auto& object : _object)
	{
		if(object) object->Terminate();
	}
	_object.clear();

	for(auto& player_base : _playerBase)
	{
		if(player_base) player_base->Terminate();
	}
	_playerBase.clear();

	for(auto& ui_base : _uiBase)
	{
		if(ui_base) ui_base->Terminate();
	}
	_uiBase.clear();

	// 個別 UI ポインタを安全に解放（存在していれば Terminate を呼ぶ）
	if(_uiHp) { _uiHp->Terminate(); _uiHp.reset(); }
	if(_uiMakimonoCnt) { _uiMakimonoCnt->Terminate(); _uiMakimonoCnt.reset(); }
	if(_henshinUi) { _henshinUi->Terminate(); _henshinUi.reset(); }
	if(_counterUi) { _counterUi->Terminate(); _counterUi.reset(); }
	if(_treasureUi) { _treasureUi->Terminate(); _treasureUi.reset(); }
	if(_attackUi) { _attackUi->Terminate(); _attackUi.reset(); }
	if(_treasureOpenUi) { _treasureOpenUi->Terminate(); _treasureOpenUi.reset(); }
	if(_dashUi) { _dashUi->Terminate(); _dashUi.reset(); }

	// チュートリアル
	for(auto& tutorial : _tutorial)
	{
		if(tutorial)
		{
			tutorial->Terminate();
		}
	}
	_tutorial.clear();

	// セーブポイント
	for(auto& sp : _savePoint)
	{
		if(sp) sp->Terminate();
	}
	_savePoint.clear();
	_lastSavedPoint = nullptr;

	// 敵（コンテナと個別カテゴリ）を確実に終了・解放
	for(auto& enemy : _enemyBase)
	{
		if(enemy) enemy->Terminate();
	}
	_enemyBase.clear();

	for(auto& e : _enemy) { if(e) e->Terminate(); }
	_enemy.clear();
	for(auto& em : _enemyMove) { if(em) em->Terminate(); }
	_enemyMove.clear();
	for(auto& ed : _enemyDog) { if(ed) ed->Terminate(); }
	_enemyDog.clear();

	// 宝箱・巻物などオブジェクト系の解放
	for(auto& t : _treasureBase) { if(t) t->Terminate(); }
	_treasureBase.clear();
	for(auto& t : _treasure) { if(t) t->Terminate(); }
	_treasure.clear();
	for(auto& tr : _treasureRapidFire) { if(tr) tr->Terminate(); }
	_treasureRapidFire.clear();

	for(auto& m : _makimono) { if(m) m->Terminate(); }
	_makimono.clear();

	// マップ等
	if(_map1) { _map1->Terminate(); _map1.reset(); }
	for(auto& mb : _mapBase)
	{
		if(mb)
		{
			mb->Terminate();
		}
	}
	_mapBase.clear();
	if(_cube) { _cube->Terminate(); _cube.reset(); }
	if(_goal) { _goal->Terminate(); _goal.reset(); }

	// オブジェクトサーバーは外部所有（ApplicationMain）なので delete しない
	_objectServer = nullptr;

	// カメラの削除を最後に行う
	if(_camera && _camera != _savedCamera)
	{
		// _cameraが_originalCameraと同じでない場合のみ削除
		delete _camera;
	}
	_camera = nullptr;

	if(_savedCamera)
	{
		delete _savedCamera;
		_savedCamera = nullptr;
	}

	// 索敵システムの終了処理
	if(_enemySensor)
	{
		_enemySensor->Terminate();
		_enemySensor.reset();
	}
	if(_sound3D)
	{
		_sound3D->StopAll();
		_sound3D.reset();
	}

	// モード内のサウンドハンドルを解放
	if(_soundFinish)
	{
		_soundFinish->Stop();
		_soundFinish = nullptr;
	}

	if(gGlobal._soundServer)
	{
		using TYPE = soundserver::SoundItemBase::TYPE;
		gGlobal._soundServer->StopType(TYPE::BGM);
		gGlobal._soundServer->StopType(TYPE::SE);
		gGlobal._soundServer->StopType(TYPE::VOICE);
		gGlobal._soundServer->StopType(TYPE::ONESHOT);
	}

	// BGM ハンドル参照をクリア（安全のため）
	_bgmInitialize = nullptr;
	_bgmChenge = nullptr;

	// その他コンテナ/状態のクリア
	_enemiesInAttackRange.clear();
	_treasureProgressMap.clear();
	_currentOpeningTreasure = nullptr;

	_isLoadComplete = false;
	_hasRenderOnce = false;
	_requestNextStage = false;
	_requestResetStage = false;

	if(_objectServer)
	{
		if(auto* map = _objectServer->GetMap())
		{
			_objectServer->DeleteObject(map);
			// ProcessInit を呼んで実際の削除（Terminate + delete）を確定させる
			_objectServer->ProcessInit();
		}
	}

	// 2) ApplicationGlobal に保持されたデータをアンロード（現在のステージのみ）
	std::string curStage = _stageManager.GetCurrentStageId();
	if(!curStage.empty())
	{
		// ステージID -> マップ名 の対応で正しいマップ名を指定してアンロードする
		std::string mapName;
		if(curStage == "Stage1") mapName = "Map1";
		else if(curStage == "Stage2") mapName = "Map2";
		else if(curStage == "Stage3") mapName = "Map3";

		if(!mapName.empty())
		{
			gGlobal.UnloadMapData(mapName);   // Map の MV1 ハンドル等を解放
		}
		gGlobal.UnloadStageData(curStage); // ステージ内 JSON 等を解放
	}

	return true;
}

void ModeGame::SuppressNextSavePointSound()
{
	// 次回セーブポイントでの効果音再生を1回だけ抑制する
	_suppressNextSavePointSound = true;
}

void ModeGame::SavePlayer(PlayerBase* player)
{
	PlayerBase* p;
	if(player)
	{
		p = player;
	}
	else
	{
		p = PlayerFactory::GetTanukiPlayer();
	}

	SaveData saveData{};
	saveData.version = 1;
	saveData.stageId = _stageManager.GetCurrentStageId();

	if(p)
	{
		saveData.playerPos = p->GetPos();
		saveData.playerDir = p->GetDir();
		saveData.makimonoCount = p->GetMakimonoCount();
	}

	// 宝箱の取得状態をセーブに含める（開いている宝箱のインデックスを保存）
	saveData.openTreasureIds.clear();
	for(size_t i = 0; i < _treasureBase.size(); ++i)
	{
		auto& tptr = _treasureBase[i];
		if(!tptr) continue;
		if(tptr->IsOpen())
		{
			saveData.openTreasureIds.push_back(static_cast<int>(i));
		}
	}

	saveData.takenMakimonoIds.clear();
	for(size_t i = 0; i < _makimono.size(); ++i)
	{
		auto& mptr = _makimono[i];
		if(!mptr) continue;
		// ワールド上で既に消えている（取得済み）ならインデックスを保存
		if(!mptr->IsVisible())
		{
			saveData.takenMakimonoIds.push_back(static_cast<int>(i));
		}
	}

	saveData.enemies.clear();
	for(auto& e : _enemyBase)
	{
		if(!e) continue;
		SaveData::EnemyInitial ei{};
		ei.enemyId = e->GetEnemyId();
		// 型判定（簡易）
		if(dynamic_cast<EnemyDog*>(e.get())) ei.type = "Dog";
		else if(dynamic_cast<EnemyMove*>(e.get())) ei.type = "EnemyMove";
		else ei.type = "Enemy";

		ei.pos = e->GetPos();
		ei.dir = e->GetDir();
		saveData.enemies.push_back(std::move(ei));
	}

	// ステージ側の patrolGroup 保存
	const ApplicationGlobal::StageData* stageData = gGlobal.GetStageData(_stageManager.GetCurrentStageId());
	if(stageData)
	{
		saveData.patrolGroups.clear();
		for(auto& kv : stageData->patrolGroup)
		{
			const std::string& gid = kv.first;
			const auto& points = kv.second; // at::vet<vec::Vec3>
			at::vet<SaveData::PatrolPoint> pts;
			int idx = 0;
			for(auto& ppos : points)
			{
				SaveData::PatrolPoint pp{};
				pp.pos = ppos;
				pp.id = idx++;
				pp.waitTime = 0.0f;
				pts.push_back(std::move(pp));
			}
			saveData.patrolGroups[gid] = std::move(pts);
		}
	}

	// 最終的にファイルへ保存
	SaveManager::Save(saveData, SaveManager::GetDefaultPath());
}

void ModeGame::ApplySaveData(const SaveData& saveData)
{
	PlayerManager::GetInstance()->SetInitialPlayerState(PlayerManager::PlayerState::TANUKI);

	if(!saveData.stageId.empty() && saveData.stageId != _stageManager.GetCurrentStageId())
	{
		_initialStageId = saveData.stageId;
	}

	if(!saveData.stageId.empty() && saveData.stageId != _stageManager.GetCurrentStageId())
	{
		_initialStageId = saveData.stageId;
	}

	// 元のカメラに戻す
	if(_savedCamera)
	{
		_camera = _savedCamera;
		_savedCamera = nullptr;
	}

	_isGameOverCinematicActive = false;

	// PlayerForm を使用してプレイヤーをセーブデータから復帰
	auto* playerForm = PlayerForm::GetInstance();
	playerForm->RestoreFromSaveData(saveData);

	PlayerBase* player = playerForm->GetPlayer();

	// メモリ上にも保存しておく（必要に応じて参照可能）
	_saveData = saveData;

	// カメラをタヌキに合わせる（即時表示性確保）
	if(_camera && player)
	{
		vec::Vec3 camDelta = vec3::VSub(_camera->GetPos(), _camera->GetTarget());
		vec::Vec3 target = vec3::VAdd(player->GetPos(), vec3::VGet(0.0f, 60.0f, 0.0f));
		_camera->SetTarget(target);
		_camera->SetPos(vec3::VAdd(target, camDelta));
	}

	int tid = 0;
	for(auto& t : _treasureBase)
	{
		if(!t)
		{
			++tid;
			continue;
		}

		bool shouldOpen = false;
		for(const auto& sid : saveData.openTreasureIds)
		{
			if(sid == tid)
			{
				shouldOpen = true;
				break;
			}
		}

		t->SetOpen(shouldOpen);

		// ゲージ進行度を必ずリセット（セーブされていない開放は復元されないようにする）
		_treasureProgressMap[t.get()] = 0.0f;

		// 連打型宝箱の進行度もリセット（死亡復帰時にセーブ時点へ戻す）
		if(auto* rapidFire = dynamic_cast<TreasureRapidFire*>(t.get()))
		{
			rapidFire->ResetCount();
		}
		++tid;
	}

	if(!saveData.takenMakimonoIds.empty() && !_makimono.empty())
	{
		for(size_t i = 0; i < _makimono.size(); ++i)
		{
			auto& mptr = _makimono[i];
			if(!mptr) continue;

			bool taken = false;
			for(auto id : saveData.takenMakimonoIds)
			{
				if(id == static_cast<int>(i))
				{
					taken = true;
					break;
				}
			}
			mptr->SetVisible(!taken);
		}
	}
	else
	{
		// 保存に情報が無ければ既定の表示（全て表示）にする
		for(auto& mptr : _makimono)
		{
			if(mptr) mptr->SetVisible(true);
		}
	}

	// 現在開けている途中状態をクリアしてゲージ残存を防ぐ
	_currentOpeningTreasure = nullptr;
	_treasureHoldSec = 0.0f;
	_isOpeningTreasure = false;

	for(auto& enemyPtr : _enemyBase)
	{
		if(!enemyPtr) continue;

		// 非巡回の敵（従来どおり初期位置へ戻す）
		enemyPtr->SetPos(enemyPtr->GetInitialPos());
		enemyPtr->SetOldPos(enemyPtr->GetInitialPos());

		// 初期向きベクトルを取得して角度を即時反映
		vec::Vec3 initDir = enemyPtr->GetInitialDir();
		if(vec3::VSize(initDir) > 0.0f)
		{
			float angle = atan2f(initDir.x * -1.0f, initDir.z * -1.0f);
			enemyPtr->SetRotationY(angle);
			enemyPtr->SetTargetRotationY(angle);
			enemyPtr->SetDir(initDir);
		}
		else
		{
			enemyPtr->SetTargetRotationFromDirection(enemyPtr->GetInitialDir());
			enemyPtr->UpdateRotation();
		}

		enemyPtr->OnPlayerLost();
		enemyPtr->ResetTeleport();
		enemyPtr->_status = CharaBase::STATUS::WAIT;
		enemyPtr->SetAlive(true);
		enemyPtr->CaptureInitialTransform();

		// 巡回中の EnemyMove は初期位置へ戻さず現在の巡回を継続させる
		if(auto* em = dynamic_cast<EnemyMove*>(enemyPtr.get()))
		{
			if(em->IsPatrolling())
			{
				// 状態だけリセットして巡回継続（位置は変更しない）
				em->OnPlayerLost();
				em->SetAlive(true);
				continue;
			}
		}

		if(auto* e = dynamic_cast<Enemy*>(enemyPtr.get()))
		{
			e->StopAnimation();
			e->PlayAnimation("idle", true);
		}
		else if(auto* em = dynamic_cast<EnemyMove*>(enemyPtr.get()))
		{
			em->StopAnimation();
			em->PlayAnimation("idle", true);
		}
	}

	_isLoadComplete = true; // ロード完了フラグを立てる

	// EffectManager を使用してエフェクトを設定
	if(player)
	{
		auto* doyaEffect = EffectManager::GetDoyaEffect();
		if(doyaEffect) doyaEffect->SetTargetPlayer(player);

		auto* nakiEffect = EffectManager::GetNakiEffect();
		if(nakiEffect) nakiEffect->SetTargetPlayer(player);

		auto* walkEffect = EffectManager::GetWalkEffect();
		if(walkEffect) walkEffect->SetPlayerPos(player);

		auto* savePointEffect = EffectManager::GetSavePointEffect();
		if(savePointEffect) savePointEffect->SetTargetPlayer(player);
	}

	// 宝箱／セーブポイント等の参照を再設定（念のため）
	auto* treasureEffect = EffectManager::GetTreasureEffect();
	if(treasureEffect) treasureEffect->SetTreasure(_treasureBase);

	auto* savePointEffect = EffectManager::GetSavePointEffect();
	if(savePointEffect) savePointEffect->SetSavePoint(_savePoint);

	auto* goalEffect = EffectManager::GetGoalEffect();
	if(goalEffect) goalEffect->SetGoal(_goal);

	// NakiEffect の一時状態をクリア
	auto* nakiEffect = EffectManager::GetNakiEffect();
	if(nakiEffect)
	{
		nakiEffect->ResetEffect();
	}

	_suppressSavePoint = false; // セーブポイントの再出現を抑制するフラグをリセット
	_suppressedSavePoint = nullptr; // 抑制中のセーブポイント参照もクリア
	if(player && !_savePoint.empty())
	{
		for(auto& sp : _savePoint)
		{
			if(!sp)
			{
				continue;
			}

			int h = sp->GetHandle();
			int f = sp->GetSavePointCollisionFrame();
			if(h > 0 || f < 0)
			{
				continue;
			}

			MATRIX model = sp->MakeModelMatrix();
			MV1SetMatrix(h, model);
			MV1RefreshCollInfo(h, f);

			vec::Vec3 hitPos;
			if(CollisionManager::GetInstance()->CheckPositionToMV1Collision(
				player->GetPos(),
				h,
				f,
				player->GetColSubY(),
				hitPos
			))
			{
				_suppressSavePoint = true;
				_suppressedSavePoint = sp.get();
				break;
			}
		}
	}
}

void ModeGame::ResetEnemyRoot()
{
	// まず保存されたセーブデータ側の patrolGroups があればそれを優先して再割当て
	if(!_saveData.patrolGroups.empty())
	{
		for(const auto& kv : _saveData.patrolGroups)
		{
			const std::string& gid = kv.first;
			const auto& points = kv.second; // at::vet<SaveData::PatrolPoint>

			// SaveData -> ApplicationGlobal::PatrolPointInfo に変換
			at::vec<ApplicationGlobal::PatrolPointInfo> ppInfos;
			ppInfos.reserve(points.size());
			for(const auto& sp : points)
			{
				ApplicationGlobal::PatrolPointInfo info;
				info.pos = sp.pos;
				info.id = sp.id;
				info.waitTime = sp.waitTime;
				ppInfos.push_back(std::move(info));
			}

			// gid と一致する EnemyMove に割り当て
			for(auto& enemyPtr : _enemyBase)
			{
				if(!enemyPtr) continue;
				if(enemyPtr->GetCustomId() != gid) continue;
				if(auto* em = dynamic_cast<EnemyMove*>(enemyPtr.get()))
				{
					em->SetPatrolPointInfo(ppInfos);
					// CaptureInitialTransform は内部で初期インデックスをキャプチャする
					em->CaptureInitialTransform();

					// 保存しておいた「初期巡回インデックス」から位置とインデックスを復元
					em->RestoreInitialPatrolPosition();

					em->OnPlayerLost();
					em->_status = CharaBase::STATUS::WAIT;
					em->SetAlive(true);
				}
			}
		}
		return;
	}

	// 以下同様に各分岐で SetPatrolPointInfo / SetPatrolPoint の後に RestoreInitialPatrolPosition() を呼ぶ
	// （patrolPointInfo 側のループ）
	const ApplicationGlobal::StageData* stageData = gGlobal.GetStageData(_stageManager.GetCurrentStageId());
	if(!stageData) return;

	for(const auto& kv : stageData->patrolPointInfo)
	{
		const std::string& gid = kv.first;
		const auto& infos = kv.second; // at::vec<ApplicationGlobal::PatrolPointInfo>

		for(auto& enemyPtr : _enemyBase)
		{
			if(!enemyPtr) continue;
			if(enemyPtr->GetCustomId() != gid) continue;
			if(auto* em = dynamic_cast<EnemyMove*>(enemyPtr.get()))
			{
				em->SetPatrolPointInfo(infos);
				em->CaptureInitialTransform();
				em->RestoreInitialPatrolPosition();
				em->OnPlayerLost();
				em->_status = CharaBase::STATUS::WAIT;
				em->SetAlive(true);
			}
		}
	}

	// フォールバック：古い patrolGroup (vec<vec::Vec3>)
	for(const auto& kv : stageData->patrolGroup)
	{
		const std::string& gid = kv.first;
		const auto& pts = kv.second;

		for(auto& enemyPtr : _enemyBase)
		{
			if(!enemyPtr) continue;
			if(enemyPtr->GetCustomId() != gid) continue;
			if(auto* em = dynamic_cast<EnemyMove*>(enemyPtr.get()))
			{
				em->SetPatrolPoint(pts);
				em->CaptureInitialTransform();
				em->RestoreInitialPatrolPosition();
				em->OnPlayerLost();
				em->_status = CharaBase::STATUS::WAIT;
				em->SetAlive(true);
			}
		}
	}

}

void ModeGame::ResetEnemiesToInitialPositions()
{
	// 敵ポインタ配列が空なら何もしない
	if(_enemyBase.empty()) return;

	for(auto& enemyPtr : _enemyBase)
	{
		if(!enemyPtr) continue;

		// EnemyMove（巡回敵）の扱い
		if(auto* em = dynamic_cast<EnemyMove*>(enemyPtr.get()))
		{
			// すでに巡回中なら位置/向きを変更せず巡回を継続させる
			if(em->IsPatrolling())
			{
				em->SetAlive(true);
				em->ResetStunEffect();
				continue;
			}

			// フォールバック：初期位置へ戻してから CaptureInitialTransform() で巡回情報を有効化する
			em->SetPos(em->GetInitialPos());
			em->SetOldPos(em->GetInitialPos());

			// Capture によって巡回が設定されるなら再開される
			em->CaptureInitialTransform();

			em->OnPlayerLost();
			em->SetAlive(true);
			em->ResetStunEffect();
			continue;
		}

		// EnemyMove 以外（従来どおり初期位置へ戻す）
		enemyPtr->SetPos(enemyPtr->GetInitialPos());
		enemyPtr->SetOldPos(enemyPtr->GetInitialPos());

		enemyPtr->OnPlayerLost();
		enemyPtr->ResetTeleport();
		enemyPtr->SetAlive(true);
		enemyPtr->CaptureInitialTransform();
		enemyPtr->ResetStunEffect();
	}
}

// 敵センサーの生成を共通化するヘルパー関数
at::spc<EnemySensor> ModeGame::CreateEnemySensor(float soundArea, MapBase* map)
{
	auto sensor = std::make_shared<EnemySensor>();
	sensor->Initialize();
	sensor->SetMap(map);
	
	return sensor;	
}

void ModeGame::CreateEnemy(
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
		enemy->SetEnemyId(nextEnemyId++);
		enemy->SetDirSequenceFromJson(object);
		enemy->SetEffect(EffectManager::GetEnemyReturnEffect());
		enemy->SetStunEffect(EffectManager::GetEnemyStunEffect());

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
		enemyMove->SetEnemyId(nextEnemyId++);
		enemyMove->SetDirSequenceFromJson(object);
		enemyMove->SetEffect(EffectManager::GetEnemyReturnEffect());
		enemyMove->SetStunEffect(EffectManager::GetEnemyStunEffect());

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

		// フォールバック
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
		sensor->SetDetectionSector(380.0f, 80.0f);

		auto dog = std::make_shared<EnemyDog>();
		dog->Initialize();
		dog->SetJsonDataUE(object);
		dog->SetEnemySensor(sensor);
		dog->SetEnemyId(nextEnemyId++);
		dog->SetEffect(EffectManager::GetEnemyReturnEffect());
		dog->SetStunEffect(EffectManager::GetEnemyStunEffect());

		// 犬の移動範囲の割り当て
		auto it = dogMovementArea.find(gid);
		if(it != dogMovementArea.end() && !it->second.empty())
		{
			dog->SetMovementArea(it->second);
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
			auto* tanuki = PlayerFactory::GetTanukiPlayer();
			tanuki->SetJsonDataUE(object);
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
			auto treasure = std::make_shared<Treasure>();
			treasure->Initialize();
			treasure->SetJsonDataUE(object);
			_treasureBase.emplace_back(treasure);
			continue;
		}

		if (name == "TreasureX")
		{
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


		if(name == "S_Savepoint")
		{
			auto sp = std::make_shared<SavePoint>();
			sp->Initialize();

			// JSON から位置等設定する関数が無ければ SetPos 等で設定
			if(object.contains("translate"))
			{
				vec::Vec3 pos;
				object.at("translate").at("x").get_to(pos.x);
				object.at("translate").at("y").get_to(pos.z);
				object.at("translate").at("z").get_to(pos.y);
				pos.z *= -1.0f;
				sp->SetPos(pos); // SavePoint が SetPos を持っている前提
			}
			// 必要なら回転/スケールも設定

			_savePoint.emplace_back(sp);
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
	if(!player || !_camera)
	{
		return false;
	}
	// カメラの位置/視点の移動を、プレイヤーの移動量に追従する
	vec::Vec3 playermove = vec3::VSub(player->GetPos(), player->GetOldPos());
	vec::Vec3 newPos = vec3::VAdd(_camera->GetPos(), playermove);
	vec::Vec3 newTarget = vec3::VAdd(_camera->GetTarget(), playermove);
	_camera->SetPos(newPos);
	_camera->SetTarget(newTarget);
	return true;
}

void ModeGame::ProcessPlayerCollision(PlayerBase* player)
{
	if(!player)
	{
		return;
	}

	auto* map = _objectServer->GetMap();
	if(!map)
	{
		return;
	}

	// 衝突判定処理
	EscapeCollision(player, map);
	CharaToTreasureHitCollision(player, _treasureBase);
	CharaToTreasureOpenCollision(player, _treasureBase);
	PlayerCameraInfo(player);
}

// 計算処理
bool ModeGame::Process()
{
	base::Process();

	// デバック用カメラ切り替え
	DebugCinematicCameraControl();

	int trg = ApplicationBase::GetInstance()->GetTrg();
	if(trg & PAD_INPUT_10)
	{
		_isGameClear = true;
		ModeServer::GetInstance()->Add(NEW ModeGameClear(this), 255, "ModeGameClear");
		return true;
	}

	ModeServer::GetInstance()->SkipProcessUnderLayer();
	ModeServer::GetInstance()->SkipRenderUnderLayer();

	if(_isGameClearCinematicActive)
	{
		ProcessClearSequence();
		AnimationManager::GetInstance()->Update(1.0f);
		return true;
	}

	if(_isGameOverCinematicActive)
	{
		ProcessGameOverSequence();
		AnimationManager::GetInstance()->Update(1.0f);
		return true;
	}

	if(_isIntroActive)
	{
		ProcessIntroSequence();
	}

	// カメラ処理
	_camera->Process();

	// デバック処理
	DebugProcess();
	DebugCameraControl();

	// アニメーション処理
	AnimationManager::GetInstance()->Update(1.0f);

	// Effekseer 更新
	EffekseerManager::GetInstance()->Update();

	// エフェクト更新
	EffectManager::UpdateAllEffect();

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
	_objectServer->ProcessInit();
	_objectServer->Process();

	// アクティブプレイヤーの取得と処理
	ProcessActivePlayer();

	// オブジェクト処理
	ObjectProcess();

	// 敵との当たり判定処理（生存している敵のみ）
	if(_d_use_collision)
	{
		CheckAllDetections();
	}

	// プレイヤーの衝突判定と処理
	ProcessPlayerCollisionAndInteraction();

	// プレイヤーと敵の接触処理
	ProcessPlayerEnemyCollision();

	// 3Dサウンド処理
	Process3DSound();

	// 変身時間制限処理
	ProcessTransformTimeLimit();

	// ゲームオーバー復帰の円形フェイドイン
	ProcessSpotlightFadeIn();

	// BGM変更処理
	ChangeBGM();

	return true;
}

// アクティブプレイヤーの取得と処理
void ModeGame::ProcessActivePlayer()
{
	// 3Dサウンドリスナー位置の設定
	PlayerBase* activePlayer = PlayerForm::GetInstance()->GetPlayer();

	if(activePlayer)
	{
		vec::Vec3 listenerPos = activePlayer->GetPos();
		vec::Vec3 listenerFront = vec3::VAdd(listenerPos, activePlayer->GetDir());
		Set3DSoundListenerPosAndFrontPos_UpVecY(
			DxlibConverter::VecToDxLib(listenerPos),
			DxlibConverter::VecToDxLib(listenerFront)
		);
	}
}

// プレイヤーの衝突判定と相互作用処理
void ModeGame::ProcessPlayerCollisionAndInteraction()
{
	PlayerBase* player = PlayerForm::GetInstance()->GetPlayer();

	if(!player)
	{
		return;
	}

	auto* map = _objectServer->GetMap();
	if(!map)
	{
		return;
	}

	// 衝突判定処理
	EscapeCollision(player, map);
	CharaToTreasureHitCollision(player, _treasureBase);
	CharaToTreasureOpenCollision(player, _treasureBase);
	PlayerCameraInfo(player);

	// オブジェクト相互作用処理
	PlayerToMakimonoCollision(player, _makimono);
	PlayerToTutorialCollision(player, _tutorial);
	PlayerToSavePointCollision(player);

	// ゴールとの当たり判定
	if(CanGoal() && !_isGameClear)
	{
		UpdateGoalConfirm(player);
	}

	// 攻撃判定は「表示中のプレイヤーが人間プレイヤーのときのみ」実行する
	if(player == PlayerFactory::GetHumanPlayer())
	{
		IsPlayerAttack(player, _enemyBase);
	}
}

// プレイヤーと敵の接触処理
void ModeGame::ProcessPlayerEnemyCollision()
{
	PlayerBase* player = PlayerForm::GetInstance()->GetPlayer();

	if(!player || !player->IsAlive())
	{
		return;
	}

	bool hasCollision = false;

	for(auto& enemy : _enemyBase)
	{
		if(!enemy || !enemy->IsAlive())
		{
			continue;
		}

		// 軽量な早期判定（XZ円）
		if(CharaToCharaCollision(player, enemy.get()))
		{
			// プレイヤーが敵に見つかっていなかったら捕まらない
			if(enemy->IsPlayerChasing())
			{
				// 犬の場合はゲームオーバーにしない
				if(dynamic_cast<EnemyDog*>(enemy.get()) != nullptr)
				{
					continue; // 犬の場合は次の敵へ
				}

				StopAllSounds();

				_hasRenderOnce = false;

				//ここでゲームオーバー処理へ移行
				StartGameOverSequence();

				break;
			}
			// 実際の押し出し（カプセル）
			hasCollision = true;
		}
	}

	// プレイヤーと敵の接触があった場合、敵の音を発生させる
	if(hasCollision && player && PlayerManager::GetInstance()->IsShowTanuki())
	{
		EnemySoundManager::GetInstance()->EmitSound(
			player->GetPos(),  // 位置
			1,					// 音の大きさレベル
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
}

// 3Dサウンド処理
void ModeGame::Process3DSound()
{
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
				// 犬の場合は専用の処理
				if(auto* dog = dynamic_cast<EnemyDog*>(eb.get()))
				{
					// 犬がプレイヤーを追跡中（FOUND状態）の場合のみ吠え声を再生
					if(dog->_status == CharaBase::STATUS::FOUND)
					{
						_sound3D->PlayLoopSound3D(eb.get(), "40", eb->GetPos());
					}
					else if(dog->_status == CharaBase::STATUS::WALK)
					{
						_sound3D->PlayLoopSound3D(eb.get(), "41", eb->GetPos());
					}
					else
					{
						// 追跡していない場合は停止
						_sound3D->StopSound3D(eb.get());
					}
				}
				// 武士（EnemyMove, Enemy）の場合
				else if(eb->_status == CharaBase::STATUS::WALK)
				{
					_sound3D->PlayLoopSound3D(eb.get(), "31", eb->GetPos());
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
}

// 変身時間制限処理
void ModeGame::ProcessTransformTimeLimit()
{
	PlayerManager* pm = PlayerManager::GetInstance();

	if(!pm->IsTransformTimeLimitActive())
	{
		return;
	}

	float dt = 1.0f / 60.0f; // 60FPS想定
	pm->UpdateTransformTimer(dt);

	// 2回攻撃時の遅延戻りタイマーを管理
	if(_tanukiReturnPending)
	{
		_tanukiReturnTimer -= dt;
		if(_tanukiReturnTimer <= 0.0f)
		{
			_tanukiReturnPending = false;
			_tanukiReturnTimer = 0.0f;

			// 攻撃アニメーションが終了したら、人間からタヌキに戻す
			pm->TransformToTanuki();
		}
	}
}

// サウンド停止ヘルパー関数
void ModeGame::StopAllSounds()
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
	SetCameraPositionAndTarget_UpVecY(
		DxlibConverter::VecToDxLib(_camera->GetPos()),
		DxlibConverter::VecToDxLib(_camera->GetTarget())
	);
	SetCameraNearFar(_camera->GetClipNear(), _camera->GetClipFar());

	const float fov_deg = 30.0f;
	SetupCamera_Perspective(DEG2RAD(fov_deg));

	// マップ（シャドウマップの持ち主）を取得
	auto* map = (_objectServer != nullptr) ? _objectServer->GetMap()   : nullptr;
	const int shadowMapHandle = (map != nullptr) ? map->GetHandleShadowMap() : -1;

	// ---- マップ側の実装に合わせたライト設定(マップと同じ処理)
	VECTOR lightdir = VGet(-1.0f, -1.0f, 0.5f);           // ライトの向き
	SetGlobalAmbientLight(GetColorF(0.f, 0.f, 0.f, 0.f)); 
	ChangeLightTypeDir(lightdir);						  

	// シャドウマップにライトの向きを設定
	SetShadowMapLightDirection(shadowMapHandle, lightdir);

	// シャドウマップに描画する範囲
	const float lenght = 800.f;			 
	DxlibConverter::SetShadowMapDrawArea
	(
		shadowMapHandle,
		vec3::VAdd(_camera->GetTarget(), vec3::VGet(-lenght, -1.0f, -lenght)),
		vec3::VAdd(_camera->GetTarget(), vec3::VGet(lenght, lenght, lenght))
	);

	// 2パス描画
	for(int path = 0; path < 2; ++path)
	{
		if(path == 0)
		{
			if(auto* map = _objectServer->GetMap())
			{
				map->SetCamera(_camera);
				map->SetExternalShadowCasters([this]() 
					{
						RenderShadowCastersFromModeGame(); 
					}
				);
			}
			ShadowMap_DrawSetup(shadowMapHandle);
			if(_objectServer) 
			{
				_objectServer->Render();
			}
		}
		else
		{
			// --- 通常描画（シャドウマップを使用） ---
			ShadowMap_DrawEnd();

			// 影あり描画（3Dモデル群）
			SetUseShadowMap(0, shadowMapHandle);

			// 通常の3D（マップ/モデル）
			if(_objectServer)
			{
				_objectServer->Render();
			}
			ObjectRender();

			// Effekseer（必ず影なしで描く）
			EffekseerManager::GetInstance()->Render();

			// ゲームオーバー演出（既存）
			if(_isGameOverCinematicActive)
			{
				SetDrawBlendMode(DX_BLENDMODE_ALPHA, _gameOverDimAlpha);
				DrawBox(0, 0, 1920, 1080, GetColor(0, 0, 0), TRUE);
				SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

				PlayerBase* tanukiPlayer = PlayerFactory::GetTanukiPlayer();
				if(tanukiPlayer)
				{
					tanukiPlayer->Render();
				}
			}

			int key = ApplicationBase::GetInstance()->GetKey();
			if(key & PAD_INPUT_12)
			{
				DebugRender();
			}
		}
	}

	bool noOverlayAbove =
		(ModeServer::GetInstance()->Get("gameover"    ) == nullptr) &&
		(ModeServer::GetInstance()->Get("gameoverload") == nullptr);

	if(noOverlayAbove)
	{
		_hasRenderOnce = true;
		_isLoadComplete = true;
	}
	// ゲームオーバー復帰時の円形フェードイン
	DrawSpotLightFadeIn();
	return true;
}

bool ModeGame::UpdateGoalConfirm(PlayerBase* player)
{
	// プレイヤーがいて、未クリア状態で、ゴールに触れているか
	const bool hitGoal = (!_isGameClear && player && PlayerToGoalHitCollision(player, _goal.get()));
	if(!hitGoal)
	{
		return false;
	}

	// 以降のゴール判定を止める（多重開始防止）
	_isGameClear = true;

	// 旧「確認UI」関連の状態は念のためクリア
	_goalConfirmOpened = false;
	_goalConfirmResult = ModeGoalConfirm::Result::None;
	_notGoalFlag = false;

	// ゴールを踏んだら即クリア演出へ
	if(!_isGameClearCinematicActive)
	{
		if(!StartClearSequence())
		{
			// 演出開始に失敗した場合はロックを戻す（詰み防止）
			_isGameClear = false;
			return false;
		}
	}

	return true;
}


// 初期ステージIDを設定
void ModeGame::SetInitialStageId(const std::string& stageId)
{
	_initialStageId = stageId;
}

