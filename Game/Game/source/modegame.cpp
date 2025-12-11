#include "modegame.h"
#include "applicationmain.h"
#include "modeeffekseer.h"

// テスト


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


	_map->SetCamera(_camera);
	_player->SetCamera(_camera);

	DebugInitialize();// デバック初期化

	_resolve_on_y = false;
	_landed_on_up = false;
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
bool ModeGame::PlayerCameraInfo()
{
	// カメラの位置/視点の移動を、プレイヤーの移動量に追従する
	VECTOR playermove = VSub(_player->GetPos(), _player->GetOldPos());
	_camera->_v_pos = VAdd(_camera->_v_pos, playermove);
	_camera->_v_target = VAdd(_camera->_v_target, playermove);
	return true;
}

// 計算処理
bool ModeGame::Process()
{
	base::Process();

	// カメラ操作
	_camera->Process();
	
	DebugProcess();// デバック処理
	
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
	for(auto& enemy : _enemy)
	{
		if(enemy->IsAlive())
		{
			CharaToCharaCollision(_player.get(), enemy.get());
			CharaToCubeCollision(enemy.get(), _cube.get());
		}
	}

	// 当たり判定の処理をここに書く
	EscapeCollision();
	CharaToCubeCollision(_player.get(), _cube.get());
	LandCheck();

	// 攻撃判定の更新処理
	UpdateCheckAttackCollision();


	// プレイヤーのカメラ情報表示
	PlayerCameraInfo();

	return true;
}

// 描画処理
bool ModeGame::Render()
{
	base::Render();

	
	// カメラ設定更新
	SetCameraPositionAndTarget_UpVecY(_camera->_v_pos, _camera->_v_target);
	SetCameraNearFar(_camera->_clip_near, _camera->_clip_far);

	// キャラを描画（生存しているもののみ）
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
