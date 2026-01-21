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
	_camera->Initialize();

	ObjectInitialize();	// オブジェクト初期化

	// キャラ
	for(auto& chara : _chara)
	{
		chara->Initialize();
	}

	// オブジェクトの初期化
	for(auto& object : _object)
	{
		object->Initialize();
	}

	// プレイヤー
	for(auto& player_base : _playerBase)
	{
		player_base->Initialize();
	}

	for(auto& treasure : _treasure)
	{
		treasure->Initialize();
	}
	// UI
	for(auto& ui_base : _uiBase)
	{
		ui_base->Initialize();
	}

	_map		 ->SetCamera(_camera);
	_player		 ->SetCamera(_camera);
	_playerTanuki->SetCamera(_camera);

	//InitHpBlock();// ブロック初期化

	DebugInitialize();// デバック初期化

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

	// エネミーにセンサーを設定
	for (auto& enemy : _enemy)
	{
		enemy->SetEnemySensor(_enemySensor);
	}

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
	for(auto& treasure : _treasure)
	{
		treasure->Terminate();
	}
	_uiBase.clear();
	delete _camera;

	// 索敵システムの終了処理
	if (_enemySensor)
	{
		_enemySensor->Terminate();
		_enemySensor.reset();
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

	// メニュー経由でカメラ編集モードが有効なら、カメラのみ操作して他は処理しない
	if(_bCameraControlMode)
	{
		int key = ApplicationMain::GetInstance()->GetKey();
		int trg = ApplicationMain::GetInstance()->GetTrg();
		const float panSpeed = 2.0f;
		const float zoomStep = 10.0f;

		// PAD_INPUT_3 を押している間は上下でズーム、左右で回転
		if(key & PAD_INPUT_3)
		{
			if(key & PAD_INPUT_UP)
			{
				CameraZoomTowardsTarget(zoomStep); // 上でズームイン（近づく）
			}
			if(key & PAD_INPUT_DOWN)
			{
				CameraZoomTowardsTarget(-zoomStep); // 下でズームアウト（離れる）
			}
			if(key & PAD_INPUT_LEFT)
			{
				// 左でターゲット回転（反時計回り）
				if(_camera) _camera->RotateAroundTarget(-0.05f);
			}
			if(key & PAD_INPUT_RIGHT)
			{
				// 右でターゲット回転（時計回り）
				if(_camera) _camera->RotateAroundTarget(0.05f);
			}
		}
		else
		{
			// 上下の向きが逆だったので反転
			if(key & PAD_INPUT_UP)
			{
				CameraMoveBy(vec3::VGet(0.0f, 0.0f, panSpeed));
			}
			if(key & PAD_INPUT_DOWN)
			{
				CameraMoveBy(vec3::VGet(0.0f, 0.0f, -panSpeed));
			}
			if(key & PAD_INPUT_LEFT)
			{
				CameraMoveBy(vec3::VGet(-panSpeed, 0.0f, 0.0f));
			}
			if(key & PAD_INPUT_RIGHT)
			{
				CameraMoveBy(vec3::VGet(panSpeed, 0.0f, 0.0f));
			}
		}

		// カメラ編集モード中は他の処理を行わず戻る
		return true;
	}

	// ここから通常のゲーム処理
	
	int trg = ApplicationMain::GetInstance()->GetTrg();
	// タヌキプレイヤー表示切替
	if(trg & PAD_INPUT_4)
	{
		_bShowTanuki = !_bShowTanuki;
		// 切り替え時に同じ場所で表示されるよう座標を同期する
		if(_bShowTanuki)
		{
			// プレイヤー→タヌキに切替：タヌキをプレイヤー位置へ
			_playerTanuki->SetPos(_player->GetPos());
			// 向きも合わせる
			_playerTanuki->SetDir(_player->GetDir());
		}
		else
		{
			// タヌキ→プレイヤーに切替：プレイヤーをタヌキ位置へ
			_player->SetPos(_playerTanuki->GetPos());
			// 向きも合わせる
			_player->SetDir(_playerTanuki->GetDir());
		}
	}

	// プレイヤーの処理（現在表示中のプレイヤーのみ）
	if(_bShowTanuki)
	{
		_playerTanuki->Process();
	}
	else
	{
		_player->Process();
	}

	// 現在のプレイヤーの位置を取得
	vec::Vec3 PlayerPos;
	if (_bShowTanuki)
	{
		PlayerPos = _playerTanuki->GetPos();
	}
	else
	{
		PlayerPos = _player->GetPos();
	}

	// キャラ処理（生存しているもののみ）
	for(auto& chara : _chara)
	{
		if(chara->IsAlive())
		{
			chara->Process();
		}
	}

	// エネミーの処理
	for (auto& enemy : _enemy)
	{
		if (enemy->IsAlive())
		{
			enemy->Process();
		}
	}

	// オブジェクト処理
	for(auto& object : _object)
	{
		object->Process();
	}

	for(auto& treasure : _treasure)
	{
		treasure->Process();
	}

	// UI処理
	for(auto& ui_base : _uiBase)
	{
		ui_base->Process();
	}

	// 敵との当たり判定処理（生存している敵のみ）
	// 	...
	// 当たり判定の処理をここに書く
	

	// EscapeCollisionはプレイヤー処理の後に呼ぶ（現在表示中のプレイヤーのみ）	
	if(_bShowTanuki)
	{
		EscapeCollision(_playerTanuki.get());
		PlayerCameraInfo(_playerTanuki.get());
	}
	else
	{
		EscapeCollision(_player.get());
		PlayerCameraInfo(_player.get());
	}

	// 索敵システムの処理
	if (_enemySensor)
	{
		_enemySensor->Process();
		CheckAllDetections();
	}

	return true;
}

// 描画処理
bool ModeGame::Render()
{
	base::Render();

	// カメラ設定更新
	SetCameraPositionAndTarget_UpVecY(VectorConverter::VecToDxLib(_camera->_vPos), VectorConverter::VecToDxLib(_camera->_vTarget));
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

// ModeGame のカメラ操作ラッパー
void ModeGame::CameraMoveBy(const vec::Vec3& delta)
{
	if(_camera)
	{
		_camera->MoveBy(delta);
	}
}

void ModeGame::CameraZoomTowardsTarget(float amount)
{
	if(_camera)
	{
		_camera->ZoomTowardsTarget(amount);
	}
}

// メニューからカメラ操作を開始する際に現在のカメラ位置を保存する
void ModeGame::StartCameraControlAndSave()
{
	if(_camera && !_hasSavedCameraState)
	{
		_savedCamPos = _camera->_vPos;
		_savedCamTarget = _camera->_vTarget;
		_hasSavedCameraState = true;
		// カメラ操作モードを有効にする
		_bCameraControlMode = true;
	}
}

// メニューからカメラ操作を終了する際に保存しておいたカメラ位置に戻す
void ModeGame::EndCameraControlAndRestore()
{
	if(_camera && _hasSavedCameraState)
	{
		_camera->_vPos = _savedCamPos;
		_camera->_vTarget = _savedCamTarget;
		_hasSavedCameraState = false;
		// カメラ操作モードを無効にする
		_bCameraControlMode = false;
	}
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

	// タヌキ状態のプレイヤーのみをチェック対象にする
	PlayerBase* currentPlayer = _playerTanuki.get();
	bool detected = _enemySensor->CheckPlayerDetection(currentPlayer);

	// 検出状態に応じてエネミーに通知
	if (detected)
	{
		vec::Vec3 playerPos = currentPlayer->GetPos();
		for (auto& enemy : _enemy)
		{
			if (enemy->IsAlive())
			{
				enemy->OnPlayerDetected(playerPos);
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

