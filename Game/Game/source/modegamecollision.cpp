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
bool ModeGame::EscapeCollision(CharaBase* chara)
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
	for(int i = 0; i < sizeof(escapeTbl) / sizeof(escapeTbl[0]); i++)
	{
		// 移動前の位置を保存
		vec::Vec3 oldvPos = chara->GetPos();
		vec::Vec3 v = chara->GetInputVector();
		vec::Vec3 oldv = v;
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

		// 移動した先でコリジョン判定
		MV1_COLL_RESULT_POLY hitPoly;

		// 主人公の腰位置から下方向への直線
		hitPoly = DxlibConverter::MV1CollCheckLine(
			_map->GetHandleMap(),
			_map->GetFrameMapCollision(),
			vec3::VAdd(chara->GetPos(), vec3::VGet(0.0f, chara->GetColSubY(), 0.0f)),
			vec3::VAdd(chara->GetPos(), vec3::VGet(0.0f, -99999.0f, 0.0f))
		);
		if(hitPoly.HitFlag)
		{
			// 当たった
			// 当たったY位置をキャラ座標にする
			vec::Vec3 tmpPos = chara->GetPos();
			tmpPos.y = hitPoly.HitPosition.y;
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
			v = oldv;
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
		VECTOR bBox = VAdd(mid, VGet(halfSize, halfSize, halfSize));
		DrawCube3D(aBox, bBox, GetColor(255, 0, 255), TRUE, FALSE);
	}

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

bool ModeGame::IsPlayerInBackSector(CharaBase* c1, PlayerBase* c2, float halfAngle, float rad)
{
	if(!c1 || !c2)
	{
		return false;
	}

	vec::Vec3 dir = c1->GetDir();// 敵の向き
	dir.y = 0.0f; // Y軸を無視

	vec::Vec3 pos = vec3::VSub(c2->GetPos(), c1->GetPos());		// 敵からプレイヤーへのベクトル
	pos.y = 0.0f; // Y軸を無視

	// 距離をチェック
	float distanceSq = pos.LengthSquare();

	if(distanceSq > rad * rad || distanceSq < 0.01f)
	{
		if(_d_view_collision)
		{
			vec::Vec3 c1pos = c1->GetPos();
			vec::Vec3 c2pos = c2->GetPos();
			DxlibConverter::DrawLine3D(
				c1pos,
				c2pos,
				GetColor(128, 128, 128)
			);
			return false;
		}
	}

	// 角度をチェック
	if(vec3::VSize(dir) < 0.01f)
	{
		return false;
	}

	// 背後方向とプレイヤー方向の正規化
	vec::Vec3 back = vec3::VScale(vec3::VNorm(dir), -1.0f);		// 敵の背後方向
	vec::Vec3 player = vec3::VNorm(pos);						// 敵からプレイヤーへの方向

	float cosTh = cosf(DEG2RAD(halfAngle));						// cos(半角)
	float dot = vec::Vec3::Dot(back, player);					// 内積計算
	bool hit = (dot > cosTh);

	if(_d_view_collision)
	{
		vec::Vec3 c1pos = c1->GetPos();// 敵の位置
		vec::Vec3 c2pos = c2->GetPos();// プレイヤーの位置

		bool hit = (dot > cosTh);

		DxlibConverter::DrawLine3D(
			c1pos,
			c2pos,
			hit ? GetColor(255, 255, 0) : GetColor(128, 128, 128)
		);

		// デバッグ用に扇形を描画
		int segment = 20;
		float centerang = atan2f(back.z, back.x); // 背後方向の角度
		float a1 = centerang - DEG2RAD(halfAngle);// 扇形の始点角度
		float a2 = centerang + DEG2RAD(halfAngle);// 扇形の終点角度

		vec::Vec3 prevpos = vec3::VGet(
			c1pos.x + cosf(a1) * rad,
			c1pos.y,
			c1pos.z + sinf(a1) * rad
		);

		for(int i = 1; i <= segment; i++)
		{
			float tang = a1 + (a2 - a1) * ((float)i / (float)segment);
			vec::Vec3 curpos = vec3::VGet(
				c1pos.x + cosf(tang) * rad,
				c1pos.y,
				c1pos.z + sinf(tang) * rad
			);
			DxlibConverter::DrawLine3D(
				prevpos,
				curpos,
				GetColor(0, 255, 0)
			);
			prevpos = curpos;
		}

		vec::Vec3 endpos = vec3::VGet(
			c1pos.x + cosf(a2) * rad,
			c1pos.y,
			c1pos.z + sinf(a2) * rad
		);
		vec::Vec3 centerpos = vec3::VGet(
			c1pos.x + cosf(a2) * rad,
			c1pos.y,
			c1pos.z + sinf(a2) * rad
		);
		DxlibConverter::DrawLine3D(
			prevpos,
			endpos,
			GetColor(0, 255, 0)
		);
		DxlibConverter::DrawLine3D(
			c1pos,
			centerpos,
			GetColor(0, 255, 0)
		);
	}

	return hit;
}