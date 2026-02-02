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



// 初期化
bool ModeGame::Initialize()
{
	if(!base::Initialize()) { return false; }

	// カメラ初期化
	_camera = new Camera();
	//_treasure.push_back(std::make_shared<Treasure>());
	//_object.emplace_back(_treasure.back());
	_camera->Initialize();

	_bShowTanuki = true;
	ObjectInitialize();	// オブジェクト初期化

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

	LoadStageData();// ステージデータ読み込み

	/*for(auto& treasure : _treasure)
	{
		treasure->Initialize();
	}*/
	// UI
	for(auto& ui_base : _uiBase)
	{
		ui_base->Initialize();
	}

	//// シャドウ
	//for(auto& charaShadow : _charaShadow)
	//{
	//	charaShadow->Initialize();
	//}

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

	_map->SetCamera(_camera);
	_player->SetCamera(_camera);
	_playerTanuki->SetCamera(_camera);

	//InitHpBlock();// ブロック初期化

	DebugInitialize();// デバック初期化
	ShadowInitialize();// シャドウ生成
	for(auto& charaShadow : _charaShadow)
	{
		charaShadow->Initialize();
	}

	_bResolveOnY = false;
	_bLandedOnUp = false;
	_bCameraControlMode = false;
	_hasSavedCameraState = false;

	
	// Effekseer 初期化
	EffekseerManager::GetInstance()->Initialize();
	_henshineffectHandle = EffekseerManager::GetInstance()->LoadEffect("res/Effect/hennsin.efkefc", 1.0f);

	_soundServer = std::make_shared<soundserver::SoundServer>();
	
	_bgmInitialize = std::make_shared<soundserver::SoundItemBGM>(mp3::shinobiashi);
	_bgmChenge = std::make_shared<soundserver::SoundItemBGM>(wav::ks010);

	_soundServer->Add("bgminitialize", _bgmInitialize.get());
	_soundServer->Add("bgmChenge", _bgmChenge.get());

	_isChengeBgm = false;

	//_bgmInitialize->Play();

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
	/*for(auto& treasure : _treasure)
	{
		treasure->Terminate();
	}*/
	
	for(auto& charaShadow : _charaShadow)
	{
		charaShadow->Terminate();
	}
	_charaShadow.clear();

	delete _camera;

	// 索敵システムの終了処理
	if (_enemySensor)
	{
		_enemySensor->Terminate();
		_enemySensor.reset();
	}
	if(_soundServer)
	{
		// 全サウンド停止
		_soundServer->StopType(soundserver::SoundItemBase::TYPE::BGM);
		_soundServer->StopType(soundserver::SoundItemBase::TYPE::SE);
		_soundServer->StopType(soundserver::SoundItemBase::TYPE::VOICE);
		_soundServer->StopType(soundserver::SoundItemBase::TYPE::ONESHOT);

		// SoundServer::Clear() は内部で delete してくれるので安全に呼ぶ
		_soundServer->Clear();

		_soundServer = nullptr;

		// 既に SoundServer::Clear() で delete 済なら二重 delete にならないよう null チェック
		_bgmInitialize = nullptr;
		_bgmChenge = nullptr;
		_isChengeBgm = false;
	}

	if(_henshineffectHandle != -1)
	{
		auto em = EffekseerManager::GetInstance();
		if(em)
		{
			em->DeleteEffect(_henshineffectHandle);
			_henshineffectHandle = -1;
		}
	}

	// Effekseer の終了（Initialize で起動していれば）
	if(_effekseerLaunched)
	{
		auto em = EffekseerManager::GetInstance();
		if(em)
		{
			em->Terminate();
			_effekseerLaunched = false;
		}
	}


	return true;
}

bool ModeGame::LoadStageData()
{
	std::string path = "res/map/";
	std::string jsonFile = "marker0128.json";
	std::string jsonObjectName = "stage";

	std::ifstream ifs(path + jsonFile);

	nlohmann::json jsonData;

	ifs >> jsonData;

	nlohmann::json stage = jsonData.at(jsonObjectName);

	for(auto& object : stage)
	{
		const std::string& name = object.at("objectName");

		if(name == "S_MarkerA")
		{
			_playerTanuki->SetJsonDataUE(object);
			continue;
		}

		if(name == "S_MarkerB")
		{
			auto enemy = std::make_shared<Enemy>();
			enemy->Initialize();
			enemy->SetJsonDataUE(object);

			// JSONのrotateは「向きベクトル」ではないので、センサー用に方向ベクトルを入れる
			//enemy->SetDir());

			// ここで「マーカー位置」を初期位置として確定
			enemy->CaptureInitialTransform();	// 敵の初期位置と向きをキャプチャ

			auto sensor = std::make_shared<EnemySensor>();				// 索敵範囲生成
			auto soundSensor = std::make_shared<EnemySoundSensor>();	// 音センサー生成

			enemy->SetEnemySensor(sensor);				// 敵に索敵範囲をセット
			enemy->SetEnemySoundSensor(soundSensor);	// 敵に音センサーをセット

			sensor->Initialize();
			sensor->SetMap(_map.get());

			soundSensor->Initialize();					// 音センサー初期化
			soundSensor->SetMap(_map.get());			// マップ設定
			soundSensor->SetSoundSensorArea(600.0f);	// 音センサー範囲
			soundSensor->SetPos(enemy->GetPos());		// 敵の位置にセット
			
			_enemy.emplace_back(enemy);	// 敵リストに追加
		}
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
	length = static_cast<float>(sqrtf(w * w + h * h));

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

	ModeServer::GetInstance()->SkipProcessUnderLayer();
	ModeServer::GetInstance()->SkipRenderUnderLayer();

	_camera->Process();

	DebugProcess();
	DebugCameraControl();

	if(_soundServer)
	{
		_soundServer->Update();
	}

	AnimationManager::GetInstance()->Update(1.0f);

	// Effekseer 更新
	EffekseerManager::GetInstance()->Update();

	PlayerTransform(); // プレイヤー変身処理
	ObjectProcess();   // オブジェクト処理
	
	// キャラクターの影処理
	for (auto& shadow : _charaShadow)
	{
		if (shadow)
		{
			shadow->Process();
		}
	}

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
		EscapeCollision(_playerTanuki.get(), _map.get());
		const bool hitTreasure = CharaToTreasureHitCollision(_playerTanuki.get(), _treasure.get());
		CharaToTreasureOpenCollision(_playerTanuki.get(), _treasure.get());
		PlayerCameraInfo(_playerTanuki.get());
	}
	else
	{
		EscapeCollision(_player.get(), _map.get());
		const bool hitTreasure = CharaToTreasureHitCollision(_player.get(), _treasure.get());
		CharaToTreasureOpenCollision(_player.get(), _treasure.get());
		PlayerCameraInfo(_player.get());
	}

	// プレイヤー vs 敵 の当たり判定（押し出し）
	CharaBase* player = _bShowTanuki
		? static_cast<CharaBase*>(_playerTanuki.get())
		: static_cast<CharaBase*>(_player.get());

	if(player && player->IsAlive())
	{
		for(auto& enemy : _enemy)
		{
			if(!enemy || !enemy->IsAlive())
			{
				continue;
			}

			// 軽量な早期判定（XZ円）
			if(IsHitCircle(player, enemy.get()))
			{
				// 実際の押し出し（カプセル）
				if(!enemy->IsShowingYouDiedMessage())
				{
					enemy->TriggerYouDiedMessage();
				}

				CharaToCharaCollision(player, enemy.get());
			}

		}
	}

	at::vec<Enemy*> enemy;
	enemy.reserve(_enemy.size());
	for(auto& e : _enemy)
	{
		enemy.push_back(e.get());
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

	IsPlayerAttack(_player.get(), enemy);

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

	//if(_henshineffectHandle != -1)
	//{
	//	if(!EffekseerManager::GetInstance()->IsPlayingEffect(_henshineffectHandle))
	//	{
	//		_henshineffectHandle = -1;
	//	}
	//}

	// キャラを描画（生存しているもののみ、プレイヤーは除外）
	for(auto& chara : _chara)
	{
		if(chara->IsAlive())
		{
			chara->Render();
		}
	}

	// 敵を描画（生存しているもののみ）
	for(auto& enemy : _enemy)
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

	//// 宝箱を描画
	//for(auto& treasure : _treasure)
	//{
	//	treasure->Render();
	//}

	// プレイヤーの描画（フラグに応じて片方のみ）
	for(auto & player_base : _playerBase)
	{
		if(_bShowTanuki)
		{
			if(player_base.get() == _playerTanuki.get() && player_base->IsAlive())
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
	for (auto& shadow : _charaShadow)
	{
		if (shadow)
		{
			shadow->Render();
		}
	}



	// UIを描画
	for(auto& ui_base : _uiBase)
	{
		ui_base->Render();
	}

	DebugRender();// デバック描画処理

	//// 敵のHP情報を画面に表示（生存している敵のみ）
	//int y_offset = 100; // 画面上部からのオフセット
	//int alive_count = 0; // 生存している敵のカウント用
	//for(int i = 0; i < _enemy.size(); i++)
	//{
	//	auto& enemy = _enemy[i];
	//	if(enemy->IsAlive())
	//	{
	//		DrawFormatString(10, y_offset + (alive_count * 20), GetColor(255, 0, 0), 
	//			"Enemy[%d] HP: %.1f / MaxHP: %.1f", 
	//			i, 
	//			enemy->GetHP(), 
	//			enemy->GetHP()); // 最大HPが分からないので現在HPを表示
	//		alive_count++;
	//	}
	//}

	// プレイヤーのHP情報も表示
	//DrawFormatString(10, 50, GetColor(0, 255, 0), 
	//	"Player HP: %.1f", _player->GetHP());

	// 索敵システムの描画
	if (_enemySensor)
	{
		_enemySensor->Render();
		_enemySensor->RenderDetectionUI();
	}

	// 各敵のセンサーを個別に描画
	for (auto& enemy : _enemy)
	{
		if (enemy->IsAlive())
		{
			// 音センサーの描画
			if (enemy->GetEnemySoundSensor())
			{
				enemy->GetEnemySoundSensor()->Render();
			}
		}
	}

	if(_d_view_collision)
	{
		//CollisionManager::GetInstance()->SetDebugDraw(true);
	}
	
	// 各敵のセンサーを個別に描画
	for (auto& enemy : _enemy)
	{
		if (enemy->IsAlive() && enemy->GetEnemySensor())
		{
			enemy->GetEnemySensor()->Render();
			enemy->GetEnemySensor()->RenderDetectionUI();
		}
	}

	// YouDiedメッセージの描画（最前面に表示）
	for (auto& enemy : _enemy)
	{
		if (enemy->IsAlive() && enemy->IsShowingYouDiedMessage())
		{
			enemy->RenderYouDiedMessage();
		}
	}

	// 宝箱を開けているメッセージ表示
	if (_isOpeningTreasure)
	{
		/*auto _playerPosx = _bShowTanuki ? _playerTanuki->GetPos().x : _player->GetPos().x;
		auto _playerPosz = _bShowTanuki ? _playerTanuki->GetPos().z : _player->GetPos().z;*/
		const char* msg = "お宝を開けています...(Aを押し続けてください)";
		int color = GetColor(255, 0, 0); // 黄色
		// 座標は適宜調整（ここでは画面左上(50, 400)に仮配置）
		DrawString(900, 500, msg, color);
	}

	// 敵を転ばせたメッセージ表示
	if(_showKnockdownMessage)
	{
		const char* msg = "敵を転ばせた";
		DrawString(900, 500, msg, GetColor(255, 255, 255));
	}

	if (anyDetected)
	{
		const char* alertMsg = "敵に発見された！変身できない！";
		// 座標は適宜調整（ここでは画面中央上部に仮配置）
		DrawString(600, 500, alertMsg, GetColor(255, 0, 0));
	}
	return true;
}

bool ModeGame::CheckAllDetections()
{
	PlayerBase* player = _bShowTanuki ? static_cast<PlayerBase*>(_playerTanuki.get())
		: static_cast<PlayerBase*>(_player.get());

	if(!player)
	{
		return false;
	}

	anyDetected = false;

	for(auto& enemy : _enemy)
	{
		if(!enemy->IsAlive())
		{
			continue;
		}

		auto sensor = enemy->GetEnemySensor();

		if(!sensor)
		{
			continue;
		}

		// センサーを敵に同期
		sensor->SetPos(enemy->GetPos());
		sensor->SetDir(enemy->GetDir());
		sensor->SetMap(_map.get());

		// タイマー更新など
		sensor->Process();

		// 音センサー処理
		auto soundSensor = enemy->GetEnemySoundSensor();

		if (soundSensor)
		{
			soundSensor->SetPos(enemy->GetPos());
			soundSensor->Process();
		}


		// タヌキ状態の時のみ検知処理を実行
		if (!_bShowTanuki)
		{
			// 人間状態では検知されない
			for (auto& enemy : _enemy)
			{
				if (enemy->IsAlive() && enemy->GetEnemySensor())
				{
					enemy->GetEnemySensor()->ResetDetection();
					enemy->OnPlayerLost();
				}
			}
			// 人間状態では変身キャンセル不要
			_bTransCancel = false;

			return true;
		}

		// プレイヤー検知チェック
		const bool detected = sensor->CheckPlayerDetection(player);	

		// プレイヤーとの距離を計算してデバッグ出力
		if(detected)
		{
			anyDetected = true;
			enemy->OnPlayerDetected(player->GetPos());
			_bTransCancel = true;
		}
		else
		{
			// 追跡中なら見失い扱いにしない（追跡を継続）
			if(!sensor->IsChasing())
			{
				enemy->OnPlayerLost();
			}
		}
	}

	// 変身キャンセルフラグを検知結果に基づいて更新
	_bTransCancel = anyDetected;

	return anyDetected;
}
