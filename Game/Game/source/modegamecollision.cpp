/*********************************************************************/
// * \file   modegamecollison.cpp
// * \brief  モードゲームクラス(当たり判定処理用)
// *
// * \author 鈴木裕稀
// * \date   2025/12/15
// * \作業内容: 新規作成 鈴木裕稀　2025/12/15
//				vec::Vec3を使用するように修正　鈴木裕稀　2026/01/17
/*********************************************************************/

#include "modegame.h"
#include "appframe.h"

// コリジョン判定で引っかかった時に、escapeTbl[]順に角度を変えて回避を試みる
bool ModeGame::EscapeCollision(CharaBase* chara, ObjectBase* obj)
{
	// プレイヤーが空中なら処理しない
	if(!chara->GetLand())
	{
		return false;
	}

	// コリジョン判定で引っかかった時に、escapeTbl[]順に角度を変えて回避を試みる
	float escapeTbl[] =
	{
		0, -10, 10, -20, 20, -30, 30, -40, 40, -50, 50, -60, 60, -70, 70, -80, 80,
	};

	// 角度を変えて回避を試みるループ
	for(int i = 0; i < sizeof(escapeTbl) / sizeof(escapeTbl[0]); i++)
	{
		// 移動前の位置を保存
		vec::Vec3 oldvPos = chara->GetPos();   // 移動前の位置を保存
		vec::Vec3 v = chara->GetInputVector(); // 移動量ベクトル
		vec::Vec3 oldv = v;					   // 移動量ベクトル保存

		// マップの情報
		auto handleMap = obj->GetModelHandleMap().begin()->second;
		auto frameMapCollision = obj->GetFrameMapCollision();

		float rad = atan2((float)v.z, (float)v.x);
		float length = chara->GetMoveSpeed() * sqrt(v.z * v.z + v.x * v.x);
		float sx = _camera->_vPos.x - _camera->_vTarget.x;
		float sz = _camera->_vPos.z - _camera->_vTarget.z;
		float camrad = atan2(sz, sx);

		// escapeTbl[i]の分だけ移動量v回転
		float escape_rad = DEG2RAD(escapeTbl[i]);
		v.x = cos(rad + camrad + escape_rad) * length;
		v.z = sin(rad + camrad + escape_rad) * length;

		// vの分移動
		chara->SetPos(vec3::VAdd(chara->GetPos(), v));

		// コリジョン処理しないならループから抜ける
		if(!_d_use_collision)
		{
			//カメラも移動する
			break;
		}

		// 主人公の腰位置から下方向への直線
		// 直接Dxlibを呼んでいた箇所を CollisionManager に置き換え
		vec::Vec3 hitPos;
		bool hit = CollisionManager::GetInstance()->CheckPositionToMV1Collision(
			chara->GetPos(),
			handleMap,
			frameMapCollision,
			chara->GetColSubY(),
			hitPos
		);
		if(hit)
		{
			// 当たった
			// 当たったY位置をキャラ座標にする
			vec::Vec3 tmpPos = chara->GetPos();
			tmpPos.y = hitPos.y;
			chara->SetPos(tmpPos);

			// キャラが上下に移動した量だけ、移動量を修正
			v.y += chara->GetPos().y - oldvPos.y;

			// ループiから抜ける
			break;
		}
		else
		{
			// 当たらなかった。元の座標に戻す
			chara->SetPos(oldvPos);
			//v = oldv;
		}
	}
	return true;
}

bool ModeGame::CharaToCharaCollision(CharaBase* c1, CharaBase* c2)
{
	if(!c1 || !c2) { return false; }

	// カプセル上下を生成（top: +Y, bottom: -Y）
	vec::Vec3 c1_pos = c1->GetPos();
	vec::Vec3 c2_pos = c2->GetPos();

	float c1_half = c1->GetColSubY();
	float c2_half = c2->GetColSubY();

	vec::Vec3 c1_top = vec3::VAdd(c1_pos, vec3::VGet(0.0f, c1_half, 0.0f));
	vec::Vec3 c1_bottom = vec3::VAdd(c1_pos, vec3::VGet(0.0f, -c1_half, 0.0f));
	vec::Vec3 c2_top = vec3::VAdd(c2_pos, vec3::VGet(0.0f, c2_half, 0.0f));
	vec::Vec3 c2_bottom = vec3::VAdd(c2_pos, vec3::VGet(0.0f, -c2_half, 0.0f));

	float c1_r = (float)c1->GetCollisionR();
	float c2_r = (float)c2->GetCollisionR();

	// カプセル同士が当たっていなければ終了
	if(!DxlibConverter::HitCheckCapsuleToCapsule(
		c1_top,
		c1_bottom,
		c1_r,
		c2_top,
		c2_bottom,
		c2_r))
	{
		return false;
	}

	// 重さの軽い方をmove、重い方をstopにする（同じなら処理しない）
	CharaBase* move = nullptr;
	CharaBase* stop = nullptr;
	if(c1->GetCollisionWeight() < c2->GetCollisionWeight())
	{
		move = c1; 
		stop = c2;
	}
	else if(c2->GetCollisionWeight() < c1->GetCollisionWeight())
	{
		move = c2; 
		stop = c1;
	}
	else
	{
		return false;
	}

	// 設定が有効なら押し出す
	if(_d_use_collision)
	{
		PushChara(move, stop);
	}

	// デバック描画
	if(_d_view_collision)
	{
		VECTOR mid = VGet(
			(stop->GetPos().x + move->GetPos().x) / 2,
			(stop->GetPos().y + move->GetPos().y) / 2,
			(stop->GetPos().z + move->GetPos().z) / 2);
		float halfSize = 8.0f;
		VECTOR aBox = VAdd(mid, VGet(-halfSize, -halfSize, -halfSize));
		VECTOR bBox = VAdd(mid, VGet( halfSize,  halfSize,  halfSize));
		DrawCube3D(aBox, bBox, GetColor(255, 0, 255), TRUE, FALSE);
	}

	return true;
}

// キャラと宝箱の当たり判定処理
bool ModeGame::CharaToTreasureHitCollision(CharaBase* chara, Treasure* treasure)
{
	// 引数チェック
	if(!chara || !treasure)
	{
		return false;
	}

	// 空中なら処理しない（設計に合わせて維持）
	if(!chara->GetLand())
	{
		return false;
	}

	// コリジョン判定で引っかかった時に、escapeTbl[]順に角度を変えて回避を試みる
	float escapeTbl[] =
	{
		0, -10, 10, -20, 20, -30, 30, -40, 40, -50, 50, -60, 60, -70, 70, -80, 80,
	};

	// 角度を変えて回避を試みるループ
	for(int i = 0; i < sizeof(escapeTbl) / sizeof(escapeTbl[0]); i++)
	{
		// 移動前の位置を保存
		vec::Vec3 oldvPos = chara->GetPos();   // 移動前の位置を保存
		vec::Vec3 v = chara->GetInputVector(); // 移動量ベクトル
		vec::Vec3 oldv = v;					   // 移動量ベクトル保存

		
		float rad = atan2((float)v.z, (float)v.x);
		float length = chara->GetMoveSpeed() * sqrt(v.z * v.z + v.x * v.x);
		float sx = _camera->_vPos.x - _camera->_vTarget.x;
		float sz = _camera->_vPos.z - _camera->_vTarget.z;
		float camrad = atan2(sz, sx);

		// escapeTbl[i]の分だけ移動量v回転
		float escape_rad = DEG2RAD(escapeTbl[i]);
		v.x = cos(rad + camrad + escape_rad) * length;
		v.z = sin(rad + camrad + escape_rad) * length;

		// vの分移動
		chara->SetPos(vec3::VAdd(chara->GetPos(), v));

		// 宝箱の指定フレームで判定
		const auto handleTreasure = treasure->GetModelHandle();
		const auto frameTreasure = treasure->GetHitCollisionFrame();

		// プレイヤーと指定したコリジョンフレームで当たり判定
		vec::Vec3 hitPos;
		const bool hit = CollisionManager::GetInstance()->CheckPositionToMV1Collision(
			chara->GetPos(),
			handleTreasure,
			frameTreasure,
			chara->GetColSubY(),
			hitPos
		);

		//　当たったから元の位置に戻す
		if(hit)
		{
			// 位置を戻して「めり込み/加速」を防ぐ
			chara->SetPos(chara->GetOldPos());
			return true;
			break;
		}
		else
		{
			// 当たらなかった。元の座標に戻す
			chara->SetPos(oldvPos);
		}
	}
	return false;
}

bool ModeGame::CharaToTreasureOpenCollision(PlayerBase* player, Treasure* treasure)
{
	// 引数チェック
	if(!player || !treasure)
	{
		return false;
	}

	// 空中なら処理しない（設計に合わせて維持）
	if(!player->GetLand())
	{
		return false;
	}
	// 宝箱の指定フレームで判定
	const auto handleTreasure = treasure->GetModelHandle();
	const auto frameTreasure  = treasure->GetOpenCollisionFrame();

	// プレイヤーと指定したコリジョンフレームで当たり判定
	vec::Vec3 hitPos;
	const bool inTreasure = CollisionManager::GetInstance()->CheckPositionToMV1Collision
	(
		player->GetPos(),
		handleTreasure,
		frameTreasure,
		player->GetColSubY(),
		hitPos
	);

	const int key = ApplicationBase::GetInstance()->GetKey();
	const bool holdA = (key & PAD_INPUT_1) != 0;

	// 条件崩れたらリセット
	if(!inTreasure || !holdA)
	{
		_treasureHoldSec = 0.0f;
		// 宝箱から離れたら取得フラグリセット
		if(!inTreasure)
		{
			_treasureTakenThisTreasure = false; // 離れたら再取得可能（”同じ宝箱で1回だけ”にしたいならここを消す）
		}
		return false;
	}

	// すでに取得済みなら何もしない
	if(_treasureTakenThisTreasure)
	{
		return false;
	}

	// 経過時間加算（固定60FPS前提：必要なら実測deltaに置換）
	const float dt = 1.0f / 60.0f;
	_treasureHoldSec += dt;

	// 1秒間ホールドで取得
	if(_treasureHoldSec >= 1.0f)
	{
		_treasureTakenCount++;
		_treasureTakenThisTreasure = true;
		_treasureHoldSec = 0.0f;

		// 宝箱状態を変えたい場合（見た目を開ける等）
		treasure->SetOpen(true);

		return true;
	}

	return false;
}


// キャラ同士の押し出し処理
bool ModeGame::PushChara(CharaBase* move, CharaBase* stop)
{
	if(!move || !stop) { return false; }

	// 移動前の位置を保存
	vec::Vec3 oldpos = move->GetPos();

	// [stop]の半径に当たらない位置まで、[move]を押し出す
	// [stop]の中心位置から、[move]の中心位置までの角度を得る
	// 角度は atan2() で求められる。ラジアン値なので注意
	float rad = atan2((float)(move->GetPos().z - stop->GetPos().z), (float)(move->GetPos().x - stop->GetPos().x));

	// [stop]の中心位置から、rad角度で [stop].r+[move].r の距離の位置に、[move]の中心位置を設定する
	float lenght = stop->GetCollisionR() + move->GetCollisionR() + 2.0f;
	vec::Vec3 newPos = move->GetPos();
	newPos.x = stop->GetPos().x + cos(rad) * lenght;
	newPos.z = stop->GetPos().z + sin(rad) * lenght;
	move->SetPos(newPos);

	// コリジョン処理するか？
	if(_d_use_collision)
	{
		// 移動した場合、マップコリジョンから出ていないか？
		MV1_COLL_RESULT_POLY hitpoly;

		// 主人公の腰位置から下方向への直線
		hitpoly = DxlibConverter::MV1CollCheckLine(
			_map->GetHandleMap(),
			_map->GetFrameMapCollision(),
			vec3::VAdd(move->GetPos(), vec3::VGet(0.0f, move->GetColSubY(), 0.0f)),
			vec3::VAdd(move->GetPos(), vec3::VGet(0.0f, -9999.f, 0.0f))
		);
		if(hitpoly.HitFlag)
		{
			// 当たったY位置をキャラ座標にする
			vec::Vec3 tmpPos = move->GetPos();
			tmpPos.y = hitpoly.HitPosition.y;
			move->SetPos(tmpPos);
		}
	}
	return true;
}

bool ModeGame::IsPlayerAttack(PlayerBase* player, at::vec<Enemy*> enemy)
{
	// 攻撃アニメ再生中なら入力を受け付けない（終了したら解除）
	if(_isTanukiAttackPlaying)
	{
		if(_tanukiAttackAnimId != -1 && AnimationManager::GetInstance()->IsPlaying(_tanukiAttackAnimId))
		{
			return false;
		}

		_isTanukiAttackPlaying = false;
		_tanukiAttackAnimId = -1;
	}

	int trg = ApplicationMain::GetInstance()->GetTrg();

	// 攻撃中は上で return 済みなので、ここに来たら新規受付OK
	if(trg & PAD_INPUT_2)
	{
		player = _playerTanuki.get();

		float halfAngle = DEG2RAD(60.0f); // 60度
		float rad = 120.0f; // 半径100

		CollisionManager::GetInstance()->SetDebugDraw(_d_view_collision);

		bool anyhit = false;

		for(auto& enemy : enemy)
		{
			if(!enemy->IsAlive())
			{
				continue;
			}

			bool hit = CollisionManager::GetInstance()->CheckSectorToPosition(
				enemy->GetPos(),
				vec3::VScale(enemy->GetDir(), -1.0f),
				rad,
				halfAngle,
				player->GetPos()
			);

			if(hit)
			{
				anyhit = true;
				enemy->PlayAnimation("walk", false);
			}
			else
			{
				enemy->PlayAnimation("wait", false);
			}
		}

		// ヒットした時だけ攻撃アニメ開始＆ロックON
		if(anyhit)
		{
			_tanukiAttackAnimId = player->PlayAnimation("gomepon_hensin", false);
			_isTanukiAttackPlaying = (_tanukiAttackAnimId != -1);
		}

		return anyhit;
	}

	return false;
}