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
	_treasure.push_back(std::make_shared<Treasure>());
	_object.emplace_back(_treasure.back());
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

	for(auto& treasure : _treasure)
	{
		treasure->Initialize();
	}
	// UI
	for(auto& ui_base : _uiBase)
	{
		ui_base->Initialize();
	}

	// シャドウ
	for(auto& charaShadow : _charaShadow)
	{
		charaShadow->Initialize();
	}

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
	ShadowInitialize();// シャドウ初期化

	_bResolveOnY = false;
	_bLandedOnUp = false;
	_bCameraControlMode = false;
	_hasSavedCameraState = false;

	_bShowTanuki = true;

	// 索敵システムの初期化
	_enemySensor = std::make_shared<EnemySensor>();
	_enemySensor->Initialize();
	_enemySensor->SetPos(vec3::VGet(200.0f, 0.0f, 200.0f)); // 適当な位置に配置
	_enemySensor->SetDir(vec3::VGet(0.0f, 0.0f, -1.0f));
	_enemySensor->SetMap(_map.get()); // マップへの参照を設定

	// エネミーにセンサーを設定
	for (auto& enemy : _enemy)
	{
		enemy->SetEnemySensor(_enemySensor);
	}



	_soundServer = std::make_shared<soundserver::SoundServer>();
	
	_bgmInitialize = std::make_shared<soundserver::SoundItemBGM>(mp3::shinobiashi);
	_bgmChenge = std::make_shared<soundserver::SoundItemBGM>(wav::ks010);

	_soundServer->Add("bgminitialize", _bgmInitialize.get());
	_soundServer->Add("bgmChenge", _bgmChenge.get());

	_isChengeBgm = false;
	_bgmInitialize->Play();

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
	for(auto& treasure : _treasure)
	{
		treasure->Terminate();
	}
	
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
	return true;
}

bool ModeGame::LoadStageData()
{
	std::string path = "res/map/";
	std::string jsonFile = "maptry.json";
	std::string jsonObjectName = "stage";

	std::ifstream ifs(path + jsonFile);

	nlohmann::json jsonData;

	ifs >> jsonData;

	nlohmann::json stage = jsonData.at(jsonObjectName);

	for(auto& object : stage)
	{
		std::string name = object.at("objectName");
		if(name == "S_MarkerA")
		{
			_playerTanuki->SetJsonDataUE(object);
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

	// カメラ操作
	_camera->Process();
	
	DebugProcess();// デバック処理
	DebugCameraControl();// デバックカメラ処理

	// メニュー経由でカメラ編集モードが有効なら、カメラのみ操作して他は処理しない
	if(_bCameraControlMode)
	{
		_soundServer->Update();
	}

	AnimationManager::GetInstance()->Update(1.0f); // アニメーション更新（仮に60FPS固定で更新）
	
	// ここから通常のゲーム処理

	PlayerTransform(); // プレイヤー変身処理
	ObjectProcess();	// オブジェクト処理
	
	
	// 敵との当たり判定処理（生存している敵のみ）
	// 	...
	// 当たり判定の処理をここに書く
	
	// エネミーとプレイヤーの当たり判定処理（現在表示中のプレイヤーのみ）
	for (auto& enemy : _enemy)
	{
		if (enemy->IsAlive())
		{
			bool hitDetected = false;
			PlayerBase* currentPlayer = nullptr;

			// 現在表示中のプレイヤーを取得
			if (_bShowTanuki && _playerTanuki && _playerTanuki->IsAlive())
			{
				currentPlayer = _playerTanuki.get();
			}
			else if (!_bShowTanuki && _player && _player->IsAlive())
			{
				currentPlayer = _player.get();
			}

			// 現在のプレイヤーとの当たり判定チェック
			if (currentPlayer && CharaToCharaCollision(currentPlayer, enemy.get()))
			{
				if (_bShowTanuki)
				{
					OutputDebugStringA("エネミーがタヌキプレイヤーに触れました！\n");
				}
				else
				{
					OutputDebugStringA("エネミーが人間プレイヤーに触れました！\n");
				}
				hitDetected = true;
			}

			// 当たり判定が発生した場合の共通処理
			if (hitDetected)
			{
				// YouDiedメッセージを表示
				enemy->TriggerYouDiedMessage();
			}
		}
	}


	// EscapeCollisionはプレイヤー処理の後に呼ぶ（現在表示中のプレイヤーのみ）	
	if(_bShowTanuki)
	{
		EscapeCollision(_playerTanuki.get(), _map.get());
		PlayerCameraInfo(_playerTanuki.get());
	}
	else
	{
		EscapeCollision(_player.get(),_map.get());
		PlayerCameraInfo(_player.get());
	}

	for(auto enemy : _enemy)
	{
		EscapeCollision(enemy.get(), _map.get());
	}

	// 索敵システムの処理
	if (_enemySensor)
	{
		_enemySensor->Process();
		CheckAllDetections();
	}

	// BGMチェンジ処理
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

	// キャラを描画（生存しているもののみ、プレイヤーは除外）
	for(auto& chara : _chara)
	{
		if(chara->IsAlive())
		{
			chara->Render();
		}
	}

	// オブジェクトを描画
	for(auto& object : _object)
	{
		object->Render();
	}

	// 宝箱を描画
	for(auto& treasure : _treasure)
	{
		treasure->Render();
	}

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


	// UIを描画
	for(auto& ui_base : _uiBase)
	{
		ui_base->Render();
	}

	DebugRender();// デバック描画処理

	// 敵のHP情報を画面に表示（生存している敵のみ）
	int y_offset = 100; // 画面上部からのオフセット
	int alive_count = 0; // 生存している敵のカウント用
	for(int i = 0; i < _enemy.size(); i++)
	{
		auto& enemy = _enemy[i];
		if(enemy->IsAlive())
		{
			DrawFormatString(10, y_offset + (alive_count * 20), GetColor(255, 0, 0), 
				"Enemy[%d] HP: %.1f / MaxHP: %.1f", 
				i, 
				enemy->GetHP(), 
				enemy->GetHP()); // 最大HPが分からないので現在HPを表示
			alive_count++;
		}
	}

	// プレイヤーのHP情報も表示
	DrawFormatString(10, 50, GetColor(0, 255, 0), 
		"Player HP: %.1f", _player->GetHP());

	// 索敵システムの描画
	if (_enemySensor)
	{
		_enemySensor->Render();
		_enemySensor->RenderDetectionUI();
	}

	// YouDiedメッセージの描画（最前面に表示）
	for (auto& enemy : _enemy)
	{
		if (enemy->IsAlive() && enemy->IsShowingYouDiedMessage())
		{
			enemy->RenderYouDiedMessage();
		}
	}

	//if(_player)
	//{
	//	int padding = 16; // フォントサイズ分の余白
	//	int block_w = 10; // 1ブロック幅
	//	int block_h = 18;  // 1ブロック高さ
	//	int gap = 4; // ブロック間の隙間

	//	int bolock = _hud

	//	int screen_w = ApplicationMain::GetInstance()->DispSizeW();
	//	int screen_h = ApplicationMain::GetInstance()->DispSizeH();

	//	int bar_x = screen_w - bar_w - padding;
	//	int bar_y = screen_h - bar_h - padding;

	//	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 192); // 半透明に設定
	//	DrawBox(bar_x, bar_y, bar_x + bar_w, bar_y + bar_h, GetColor(40, 40, 40), TRUE); // 黒い背景
	//	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0); // ブレンドモード解除

	//	float hp = _player->GetHP();

	//}

	return true;
}

// 全てのエネミーに対してプレイヤー検出をチェック
bool ModeGame::CheckAllDetections()
{
	if (!_enemySensor)
	{
		return false;
	}

	// タヌキ状態の時のみ検知処理を実行
	if (!_bShowTanuki)
	{
		// 人間状態では検知されない
		// 検知状態をリセットして敵に状態変更を通知
		for (auto& enemy : _enemy)
		{
			if (enemy->IsAlive())
			{
				enemy->OnPlayerLost();
			}
		}
		return true;
	}

	// 各敵個別に初期位置に戻り中かをチェック
	// タヌキ状態のプレイヤーのみをチェック対象にする
	PlayerBase* currentPlayer = _playerTanuki.get();
	bool detected = _enemySensor->CheckPlayerDetection(currentPlayer);

	// 検出状態に応じてエネミーに通知（ただし、戻り中の敵は除く）
	if (detected)
	{
		vec::Vec3 playerPos = currentPlayer->GetPos();// 検出したプレイヤーの位置を取得

		// プレイヤーが検出範囲内に入った場合
		for (auto& enemy : _enemy)
		{
			if (enemy->IsAlive())
			{
				// 個別に初期位置に戻り中でない敵のみに通知
				if (!enemy->IsReturningToInitialPosition())
				{
					enemy->OnPlayerDetected(playerPos);
				}
			}
		}
	}
	else
	{
		// プレイヤーが検出範囲外になった場合
		for (auto& enemy : _enemy)
		{
			if (enemy->IsAlive())
			{
				enemy->OnPlayerLost();
			}
		}
	}

	return detected;
}

