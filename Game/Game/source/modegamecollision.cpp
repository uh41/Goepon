/*********************************************************************/
// * \file   modegamecollison.cpp
// * \brief  モードゲームクラス(当たり判定処理用)
// *
// * \author 鈴木裕稀
// * \date   2025/12/15
// * \作業内容: 新規作成 鈴木裕稀　2025/12/15
/*********************************************************************/

#include "modegame.h"

// コリジョン判定で引っかかった時に、escapeTbl[]順に角度を変えて回避を試みる
bool ModeGame::EscapeCollision(PlayerBase* player)
{
	// プレイヤーが空中なら処理しない
	if(!player->GetLand())
	{
		return false;
	}

	// コリジョン判定で引っかかった時に、escapeTbl[]順に角度を変えて回避を試みる
	float escapeTbl[] =
	{
		0, -10, 10, -20, 20, -30, 30, -40, 40, -50, 50, -60, 60, -70, 70, -80, 80,
	};
	for(int i = 0; i < sizeof(escapeTbl) / sizeof(escapeTbl[0]); i++)
	{
		// 移動前の位置を保存
		VECTOR oldvPos = player->GetPos();
		VECTOR v = player->GetInputVector();
		VECTOR oldv = v;
		float rad = atan2((float)v.z, (float)v.x);
		float length = player->GetMoveSpeed() * sqrt(v.z * v.z + v.x * v.x);
		float sx = _camera->_vPos.x - _camera->_vTarget.x;
		float sz = _camera->_vPos.z - _camera->_vTarget.z;
		float camrad = atan2(sz, sx);

		// escapeTbl[i]の分だけ移動量v回転
		float escape_rad = DEG2RAD(escapeTbl[i]);
		v.x = cos(rad + camrad + escape_rad) * length;
		v.z = sin(rad + camrad + escape_rad) * length;

		// vの分移動
		player->SetPos(VAdd(player->GetPos(), v));

		// コリジョン処理しないならループから抜ける
		if(!_d_use_collision)
		{
			//カメラも移動する
			break;
		}

		// 移動した先でコリジョン判定
		MV1_COLL_RESULT_POLY hitPoly;

		// 主人公の腰位置から下方向への直線
		hitPoly = MV1CollCheck_Line(_map->GetHandleMap(), _map->GetFrameMapCollision(),
			VAdd(player->GetPos(), VGet(0, player->GetColSubY(), 0)), VAdd(player->GetPos(), VGet(0, -99999.f, 0)));
		if(hitPoly.HitFlag)
		{
			// 当たった
			// 当たったY位置をキャラ座標にする
			VECTOR tmpPos = player->GetPos();
			tmpPos.y = hitPoly.HitPosition.y;
			player->SetPos(tmpPos);

			// キャラが上下に移動した量だけ、移動量を修正
			v.y += player->GetPos().y - oldvPos.y;

			// ループiから抜ける
			break;
		}
		else
		{
			// 当たらなかった。元の座標に戻す
			player->SetPos(oldvPos);
			v = oldv;
		}
	}
	return true;
}

bool ModeGame::CharaToCharaCollision(CharaBase* c1, CharaBase* c2)
{
	if(!c1 || !c2) { return false; }

	// カプセル上下を生成（top: +Y, bottom: -Y）
	VECTOR c1_pos = c1->GetPos();
	VECTOR c2_pos = c2->GetPos();

	float c1_half = c1->GetColSubY();
	float c2_half = c2->GetColSubY();

	VECTOR c1_top = VAdd(c1_pos, VGet(0.0f, c1_half, 0.0f));
	VECTOR c1_bottom = VAdd(c1_pos, VGet(0.0f, -c1_half, 0.0f));
	VECTOR c2_top = VAdd(c2_pos, VGet(0.0f, c2_half, 0.0f));
	VECTOR c2_bottom = VAdd(c2_pos, VGet(0.0f, -c2_half, 0.0f));

	float c1_r = (float)c1->GetCollisionR();
	float c2_r = (float)c2->GetCollisionR();

	// カプセル同士が当たっていなければ終了
	if(!HitCheck_Capsule_Capsule(c1_top, c1_bottom, c1_r, c2_top, c2_bottom, c2_r))
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
		VECTOR bBox = VAdd(mid, VGet(halfSize, halfSize, halfSize));
		DrawCube3D(aBox, bBox, GetColor(255, 0, 255), TRUE, FALSE);
	}

	return true;
}

bool ModeGame::CharaToCubeCollision(CharaBase* chara, Cube* cube)
{
	if(!chara || !cube)
	{
		return false;
	}

	if(cube->GetVertexCount() == 0)
	{
		return false;
	}

	if(!_d_use_collision && chara == _player.get())
	{
		return false;
	}

	_bResolveOnY = false;
	_bLandedOnUp = false;

	mymath::AABB box = cube->GetAABB();
	VECTOR pos = chara->GetPos();
	float r = (float)chara->GetCollisionR();

	if(!IsHitSphereAABB(pos, r, box))
	{
		return false;
	}

	// デバッグ表示：プレイヤー位置と AABB の最短点を線で描画し、AABB を可視化する
	if(_d_view_collision)
	{
		// AABB の最短点 (clamp)
		VECTOR closest = pos;
		if(closest.x < box.min.x) closest.x = box.min.x;
		else if(closest.x > box.max.x) closest.x = box.max.x;
		if(closest.y < box.min.y) closest.y = box.min.y;
		else if(closest.y > box.max.y) closest.y = box.max.y;
		if(closest.z < box.min.z) closest.z = box.min.z;
		else if(closest.z > box.max.z) closest.z = box.max.z;

		// AABB をワイヤー/薄緑で描画（デバッグ）
		DrawCube3D(box.min, box.max, GetColor(0, 200, 0), FALSE, FALSE);

		// プレイヤー中心から最短点までの線を赤で描画
		DrawLine3D(pos, closest, GetColor(255, 0, 0));

		// 最短点に小さな赤い立方体を置いて当たり点を表示
		float markerHalf = 0.25f;
		VECTOR aMarker = VAdd(closest, VGet(-markerHalf, -markerHalf, -markerHalf));
		VECTOR bMarker = VAdd(closest, VGet(markerHalf, markerHalf, markerHalf));
		DrawCube3D(aMarker, bMarker, GetColor(255, 0, 0), TRUE, FALSE);
	}

	float dxmin = box.min.x - (pos.x + r);
	float xmin = dxmin * dxmin;
	float dxmax = box.max.x - (pos.x - r);
	float xmax = dxmax * dxmax;
	float dymin = box.min.y - (pos.y + r);
	float ymin = dymin * dymin;
	float dymax = box.max.y - (pos.y - r);
	float ymax = dymax * dymax;
	float dzmin = box.min.z - (pos.z + r);
	float zmin = dzmin * dzmin;
	float dzmax = box.max.z - (pos.z - r);
	float zmax = dzmax * dzmax;

	float resolverx;// x方向の押し出し量
	if(xmin < xmax)
	{
		resolverx = dxmin;
	}
	else
	{
		resolverx = dxmax;
	}

	float resolvery;
	if(ymin < ymax)
	{
		resolvery = dymin;
	}
	else
	{
		resolvery = dymax;
	}

	float resolverz;
	if(zmin < zmax)
	{
		resolverz = dzmin;
	}
	else
	{
		resolverz = dzmax;
	}

	float absx = resolverx * resolverx;
	float absy = resolvery * resolvery;
	float absz = resolverz * resolverz;

	// 一番小さい押し出し量でキャラを押し出す
	if(absx <= absy && absx <= absz)
	{
		pos.x += resolverx;
	}
	else if(absy <= absx && absy <= absz)
	{
		pos.y += resolvery;
		_bResolveOnY = true;
		float land_y = (resolvery - dymax) * (resolvery - dymax);
		if(land_y < !0.0f)
		{
			_bLandedOnUp = true;
		}
	}
	else if(absz <= absx && absz <= absy)
	{
		pos.z += resolverz;
	}

	chara->SetPos(pos);

	if(_d_use_collision && _bResolveOnY && _bLandedOnUp)
	{
		_player->SetLand(true);
		// キューブの上面に着地した場合、Y座標をキューブの上面に合わせる
		// 必要に応じてオフセット値を調整可能
		VECTOR tmpPos = chara->GetPos();
		float y_offset = 0.0f;  // キューブの上からさらに下にずらしたい場合は負の値を設定
		tmpPos.y = box.max.y + y_offset;
		chara->SetPos(tmpPos);
	}

	if(_d_use_collision)
	{
		MV1_COLL_RESULT_POLY hitpoly;
		hitpoly = MV1CollCheck_Line(
			_map->GetHandleMap(),
			_map->GetFrameMapCollision(),
			VAdd(chara->GetPos(), VGet(0, chara->GetColSubY(), 0)),
			VAdd(chara->GetPos(), VGet(0, -9999.f, 0))
		);
		if(hitpoly.HitFlag)
		{
			float ground_y = hitpoly.HitPosition.y;

			if(_bLandedOnUp)
			{
				//当たったY位置をキャラ座標にする
				VECTOR tmpPos = chara->GetPos();
				tmpPos.y = hitpoly.HitPosition.y;
				chara->SetPos(tmpPos);
				_player->SetLand(true);
			}
			else
			{
				if(chara->GetPos().y > ground_y)
				{
					_player->SetLand(false);
				}
				else
				{
					VECTOR tmpPos = chara->GetPos();
					tmpPos.y = ground_y;
					chara->SetPos(tmpPos);
					_player->SetLand(true);
				}
			}
			return true;
		}
	}
	return false;
}

bool ModeGame::LandCheck()
{
	if(_player->GetLand())
	{
		bool is_ground = false;

		VECTOR pos = _player->GetPos();

		// 地面にいるか？
		if(pos.y <= 0.0f)
		{
			is_ground = true;
		}

		// マップにいるか？
		if(!is_ground)
		{
			VECTOR start = VAdd(pos, VGet(0.0f, 10.0f, 0.0f));
			VECTOR end = VAdd(pos, VGet(0.0f, -50.0f, 0.0f));

			MV1_COLL_RESULT_POLY hitpoly;
			hitpoly = MV1CollCheck_Line(
				_map->GetHandleMap(),
				_map->GetFrameMapCollision(),
				start,
				end
			);
			if(hitpoly.HitFlag)
			{
				is_ground = true;
			}
		}

		// キューブの上にいるか？ - _landed_on_upフラグで判定
		if(!is_ground && _d_use_collision)
		{
			is_ground = _bLandedOnUp;
		}

		// どの足場にも乗っていなければ、空中状態にする
		if(!is_ground)
		{
			_player->SetLand(false);
		}
	}
	return false;
}

// キャラ同士の押し出し処理
bool ModeGame::PushChara(CharaBase* move, CharaBase* stop)
{
	if(!move || !stop) { return false; }

	// 移動前の位置を保存
	VECTOR oldpos = move->GetPos();

	// [stop]の半径に当たらない位置まで、[move]を押し出す
	// [stop]の中心位置から、[move]の中心位置までの角度を得る
	// 角度は atan2() で求められる。ラジアン値なので注意
	float rad = atan2((float)(move->GetPos().z - stop->GetPos().z), (float)(move->GetPos().x - stop->GetPos().x));

	// [stop]の中心位置から、rad角度で [stop].r+[move].r の距離の位置に、[move]の中心位置を設定する
	float lenght = stop->GetCollisionR() + move->GetCollisionR() + 2.0f;
	VECTOR newPos = move->GetPos();
	newPos.x = stop->GetPos().x + cos(rad) * lenght;
	newPos.z = stop->GetPos().z + sin(rad) * lenght;
	move->SetPos(newPos);

	// コリジョン処理するか？
	if(_d_use_collision)
	{
		// 移動した場合、マップコリジョンから出ていないか？
		MV1_COLL_RESULT_POLY hitpoly;

		// 主人公の腰位置から下方向への直線
		hitpoly = MV1CollCheck_Line(
			_map->GetHandleMap(),
			_map->GetFrameMapCollision(),
			VAdd(move->GetPos(), VGet(0, move->GetColSubY(), 0)), VAdd(move->GetPos(), VGet(0, -9999.f, 0))
		);
		if(hitpoly.HitFlag)
		{
			// 当たったY位置をキャラ座標にする
			VECTOR tmpPos = move->GetPos();
			tmpPos.y = hitpoly.HitPosition.y;
			move->SetPos(tmpPos);
		}
	}
	return true;
}

// ---------------------------------------------------------
// 攻撃判定の更新・追従・衝突チェックを一括で行う
// ---------------------------------------------------------
bool ModeGame::UpdateCheckAttackCollision()
{
	// 攻撃機能は削除されました
	return true;
}