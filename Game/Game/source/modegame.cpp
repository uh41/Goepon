/*********************************************************************/
// * \file   modegame.cpp
// * \brief  モードゲームクラス
// *
// * \author 鈴木裕稀
// * \date   2025/12/15
// * \作業内容: 新規作成 鈴木裕稀　2025/12/15
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

	_map->SetCamera(_camera);
	_player->SetCamera(_camera);
	_playerTanuki->SetCamera(_camera);

	DebugInitialize();// デバック初期化

	_bResolveOnY = false;
	_bLandedOnUp = false;
	_bCameraControlMode = false;
	_hasSavedCameraState = false;

	_bShowTanuki = false;
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
	delete _camera;
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
	VECTOR playermove = VSub(player->GetPos(), player->GetOldPos());
	_camera->_vPos = VAdd(_camera->_vPos, playermove);
	_camera->_vTarget = VAdd(_camera->_vTarget, playermove);
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
				CameraMoveBy(VGet(0.0f, 0.0f, panSpeed));
			}
			if(key & PAD_INPUT_DOWN)
			{
				CameraMoveBy(VGet(0.0f, 0.0f, -panSpeed));
			}
			if(key & PAD_INPUT_LEFT)
			{
				CameraMoveBy(VGet(-panSpeed, 0.0f, 0.0f));
			}
			if(key & PAD_INPUT_RIGHT)
			{
				CameraMoveBy(VGet(panSpeed, 0.0f, 0.0f));
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

	// キャラ処理（生存しているもののみ）
	for(auto& chara : _chara)
	{
		if(chara->IsAlive())
		{
			chara->Process();
		}
	}

	// オブジェクト処理
	for(auto& object : _object)
	{
		object->Process();
	}

	// 敵との当たり判定処理（生存している敵のみ）
// 	...
	// 当たり判定の処理をここに書く
	
	CharaToCubeCollision(_player.get(), _cube.get());
	LandCheck();

	// 攻撃判定の更新処理
	UpdateCheckAttackCollision();

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

	return true;
}

// 描画処理
bool ModeGame::Render()
{
	base::Render();

	
	// カメラ設定更新
	SetCameraPositionAndTarget_UpVecY(_camera->_vPos, _camera->_vTarget);
	SetCameraNearFar(_camera->_fClipNear, _camera->_fClipFar);

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

	return true;
}

// 追加: ModeGame のカメラ操作ラッパー
void ModeGame::CameraMoveBy(const VECTOR& delta)
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
