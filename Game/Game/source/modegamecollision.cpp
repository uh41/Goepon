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
	// 無効チェック
	if(!chara || !obj) { return false; }

	// プレイヤーが空中なら処理しない
	if(!chara->GetLand())
	{
		return false;
	}

	auto handleMap = obj->GetModelHandleMap();
	if(handleMap.empty()) { return false; }

	// モデルハンドル取得
	const int modelHandle    = handleMap.begin()->second;
	const int frameIndex     = obj->GetFrameMapCollision();
	if(modelHandle < 0 || frameIndex < 0) { return false; }

	vec::Vec3 oldPos = chara->GetPos();

	// コリジョン判定で引っかかった時に、escapeTbl[]順に角度を変えて回避を試みる
	float escapeTbl[] =
	{
		0, -10, 10, -20, 20, -30, 30, -40, 40, -50, 50, -60, 60, -70, 70, -80, 80,
	};
	for(int i = 0; i < sizeof(escapeTbl) / sizeof(escapeTbl[0]); i++)
	{
		// 現在フレームの移動量
		vec::Vec3 v = chara->GetMoveV();

		// カメラ角補正（元コード踏襲）
		const float rad = atan2(static_cast<float>(v.z), static_cast<float>(v.x));
		const float length = chara->GetMoveSpeed() * sqrt(v.z * v.z + v.x * v.x);

		float camrad = 0.0f;
		if(_camera)
		{
			const float sx = _camera->_vPos.x - _camera->_vTarget.x;
			const float sz = _camera->_vPos.z - _camera->_vTarget.z;
			camrad = atan2(sz, sx);
		}

		const float escape_rad = DEG2RAD(escapeTbl[i]);
		v.x = cos(rad + camrad + escape_rad) * length;
		v.z = sin(rad + camrad + escape_rad) * length;

		// 候補位置へ移動
		vec::Vec3 candidate = vec3::VAdd(oldPos, v);
		chara->SetPos(candidate);

		// コリジョン無効ならこの時点で終わり
		if(!_d_use_collision)
		{
			return true;
		}

		// ---- ここが「貼ってくれた真下レイ判定」と同等 ----
		const float colSubY = chara->GetColSubY();

		const vec::Vec3 start = vec3::VAdd(candidate, vec3::VGet(0.0f, colSubY, 0.0f));
		const vec::Vec3 end = vec3::VAdd(candidate, vec3::VGet(0.0f, -99999.0f, 0.0f));

		MV1_COLL_RESULT_POLY hitPoly = DxlibConverter::MV1CollCheckLine(
			modelHandle,
			frameIndex,
			start,
			end
		);

		if(hitPoly.HitFlag)
		{
			// 床に吸着（Y確定）
			candidate.y = hitPoly.HitPosition.y;
			chara->SetPos(candidate);

			// 必要なら v.y 更新（既存踏襲：旧posとの差分を足す）
			// ※ oldPos からの差分にするなら candidate.y - oldPos.y
			v.y += (candidate.y - oldPos.y);

			return true;
		}

		// 床が無い＝不採用 → 元に戻して次候補へ
		chara->SetPos(oldPos);
	}

	// 全候補失敗
	chara->SetPos(oldPos);
	return false;
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
	// 無効チェック
	//if(!chara || !treasure || treasure->IsOpen() || !_d_use_collision)
	if(!chara || !treasure || treasure->IsOpen())
	{
		return false;
	}

	// 宝箱のモデル/フレーム有効チェック
	int modelHandle = treasure->GetModelHandle();
	int frameIndex  = treasure->GetHitCollisionFrame();

	// 無効なら終了
	if(modelHandle < 0 || frameIndex < 0)
	{
		return false;
	}
	
	const vec::Vec3 tpos = treasure->GetPos();
	MV1SetPosition(modelHandle, DxlibConverter::VecToDxLib(tpos));

	// キャラクターの現在位置と半径
	const vec::Vec3 currentPos = chara->GetPos();
	float rad  = static_cast<float>(chara->GetCollisionR());
	float half = chara->GetColSubY();

	// カプセルの上下端を計算
	vec::Vec3 capTop    = vec3::VAdd(currentPos, vec3::VGet(0.0f,  half, 0.0f));
	vec::Vec3 capBottom = vec3::VAdd(currentPos, vec3::VGet(0.0f, -half, 0.0f));

	//宝の当たり判定フレームに対してカプセル判定
	MV1_COLL_RESULT_POLY_DIM hit;
	hit = DxlibConverter::HitCapsuleToCollision
	(
		modelHandle,
		frameIndex,
		capTop,
		capBottom,
		rad
	);

	// ヒットしなかった
	if(hit.HitNum == 0)
	{
		if(_d_view_collision)
		{
			VECTOR center = DxlibConverter::VecToDxLib(currentPos);
			float halfSize = max(rad, half); 
			VECTOR aBox = VAdd(center, VGet(-halfSize, -halfSize, -halfSize));
			VECTOR bBox = VAdd(center, VGet( halfSize,  halfSize,  halfSize));
			DrawCube3D(aBox, bBox, GetColor(255, 0, 0), TRUE, TRUE);
		}
		
		MV1CollResultPolyDimTerminate(hit); // メモリ開放
		return false;
	}

	// ヒットしたので元の位置に戻す
	// まず現在位置を取得（カメラ補正用）
	
	// 衝突時は旧位置へ戻す
	const vec::Vec3 oldPos = chara->GetOldPos();
	chara->SetPos(oldPos);

	if (_d_view_collision)
	{
		VECTOR center = DxlibConverter::VecToDxLib(currentPos);
		float halfSize = max(rad, half);
		VECTOR aBox = VAdd(center, VGet(-halfSize, -halfSize, -halfSize));
		VECTOR bBox = VAdd(center, VGet(halfSize, halfSize, halfSize));
		DrawCube3D(aBox, bBox, GetColor(0, 255, 0), TRUE, TRUE);
	}

	// カメラの補正
	if(_camera)
	{
		vec::Vec3 canDelta = vec3::VSub(oldPos, currentPos);
		_camera->_vPos = vec3::VAdd(_camera->_vPos, canDelta);
		_camera->_vTarget = vec3::VAdd(_camera->_vTarget, canDelta);
	}

	// メモリ開放
	MV1CollResultPolyDimTerminate(hit);
	return true;
}

bool ModeGame::CharaToTreasureOpenCollision(CharaBase* chara, Treasure* treasure)
{
	// 無効チェック
	if (!chara || !treasure || treasure->IsOpen())
	{
		return false;
	}

	// 宝箱のモデルとフレーム有効チェック	
	int modelHandle = treasure->GetModelHandle();
	int frameIndex  = treasure->GetHitCollisionFrame();


	// 無効なら終了
	if (modelHandle < 0 || frameIndex < 0)
	{
		return false;
	}
	// キャラクターの現在位置と半径
	const vec::Vec3 currentPos = chara->GetPos();
	float rad = static_cast<float>(chara->GetCollisionR());
	float half = chara->GetColSubY();

	// カプセルの上下端を計算
	vec::Vec3 capTop    = vec3::VAdd(currentPos, vec::Vec3(0.0f, half, 0.0f));
	vec::Vec3 capBottom = vec3::VAdd(currentPos, vec::Vec3(0.0f, -half, 0.0f));

	//宝の当たり判定フレームに対してカプセル判定
	MV1_COLL_RESULT_POLY_DIM hit;
	hit = DxlibConverter::HitCapsuleToCollision
	(
		modelHandle,
		frameIndex,
		capTop,
		capBottom,
		rad
	);

	// ヒットしなかった
	if(hit.HitNum = 0)
	{
		MV1CollResultPolyDimTerminate(hit);
		return false;
	}

	// ヒットしたのでAボタンで宝箱を開ける
	if(CheckHitKey(KEY_INPUT_A))
	{
		treasure->SetOpen(true);
	}
	
	MV1CollResultPolyDimTerminate(hit);
	return true;
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
	int trg = ApplicationMain::GetInstance()->GetTrg();

	if(trg & PAD_INPUT_2)
	{
		player = _playerTanuki.get();

		float halfAngle = DEG2RAD(60.0f); // 60度
		float rad = 120.0f; // 半径100

		for(auto& enemy : enemy)
		{
			if(!enemy->IsAlive()) {
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
				player->PlayAnimation("goepon_walk", false);
				enemy->PlayAnimation("walk", false);
			}
		}
	}

	return true;
}