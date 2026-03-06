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

bool ModeGame::EscapeCollision(CharaBase* chara, ObjectBase* obj)
{
	// プレイヤーが空中なら処理しない
	if(!chara->GetLand())
	{
		return false;
	}

	float escapeTbl[] =
	{
		0, -10, 10, -20, 20, -30, 30, -40, 40, -50, 50, -60, 60, -70, 70, -80, 80,
	};

	// マップの情報（obj が Map なら取得、なければ ModeGame::_map を使う）
	MapBase* map = dynamic_cast<MapBase*>(obj);

	for(int i = 0; i < sizeof(escapeTbl) / sizeof(escapeTbl[0]); i++)
	{
		vec::Vec3 oldvPos = chara->GetPos();
		vec::Vec3 v = chara->GetInputVector();

		float rad = atan2((float)v.z, (float)v.x);
		float length = chara->GetMoveSpeed() * sqrt(v.z * v.z + v.x * v.x);
		float sx = _camera->GetPos().x - _camera->GetTarget().x;
		float sz = _camera->GetPos().z - _camera->GetTarget().z;
		float camrad = atan2(sz, sx);

		float escape_rad = DEG2RAD(escapeTbl[i]);
		// 回転を適用した移動ベクトル（ワールド空間）
		vec::Vec3 triedV;
		triedV.x = cos(rad + camrad + escape_rad) * length;
		triedV.z = sin(rad + camrad + escape_rad) * length;
		triedV.y = 0.0f;

		// 移動（試行）
		chara->SetPos(vec3::VAdd(chara->GetPos(), triedV));

		// 早期脱出フラグ
		if(!_d_use_collision)
		{
			break;
		}

		// 当たり判定（Map がある場合は各ブロックの collisionFrame を使う）
		vec::Vec3 hitPos;
		bool hit = false;

		if(map)
		{
			auto& list = map->GetBlockPosList();

			for(size_t bi = 0; bi < list.size(); ++bi)
			{
				auto& block = list[bi];

				// Checkするループ内、ブロック情報を出力する箇所を拡張
				if(block.modelHandle >= 0)
				{
					// transform を確実に適用
					MV1SetPosition(block.modelHandle, VGet(block.x, block.y, block.z));
					MV1SetRotationXYZ(block.modelHandle, VGet(block.rx, block.ry, block.rz));
					MV1SetScale(block.modelHandle, VGet(block.sx, block.sy, block.sz));
					MV1RefreshCollInfo(block.modelHandle, block.collisionFrame);
				}

				// 必要な条件を満たさない場合はスキップ
				if(block.modelHandle < 0) continue;
				if(block.collisionFrame < 0) continue;

				if(CollisionManager::GetInstance()->CheckPositionToMV1Collision(
					chara->GetPos(),
					block.modelHandle,
					block.collisionFrame,
					chara->GetColSubY(),
					hitPos))
				{
					hit = true;
					break;
				}
			}
		}

		if(hit)
		{
			vec::Vec3 tmpPos = chara->GetPos();
			tmpPos.y = hitPos.y;
			chara->SetPos(tmpPos);

			// Y変化を移動ベクトルに反映（必要なら）
  			triedV.y += chara->GetPos().y - oldvPos.y;

			break;
		}
		else
		{
			// 当たらなかった。元の座標に戻す
			chara->SetPos(oldvPos);
		}
	}

	return true;
}

bool ModeGame::CharaToCharaCollision(CharaBase* c1, CharaBase* c2)
{
	if(!c1 || !c2) { return false; }

	if(_isTanukiAttackPlaying)
	{
		// タヌキアタック中は当たり判定を無効化
		return false;
	}

	// 追加: いずれかが EnemyBase にキャストできてスタン中であれば当たり判定を無視
	if(auto e1 = dynamic_cast<EnemyBase*>(c1))
	{
		if(e1->IsStun()) return false;
	}
	if(auto e2 = dynamic_cast<EnemyBase*>(c2))
	{
		if(e2->IsStun()) return false;
	}

	// カプセル上下を生成（top: +Y, bottom: -Y）
	vec::Vec3 c1_pos = c1->GetPos();
	vec::Vec3 c2_pos = c2->GetPos();

	float c1_half = c1->GetColSubY();
	float c2_half = c2->GetColSubY();

	vec::Vec3 c1_top = vec3::VAdd(c1_pos, vec3::VGet(0.0f, c1_half, 0.0f));
	vec::Vec3 c1_bottom = vec3::VAdd(c1_pos, vec3::VGet(0.0f, -c1_half, 0.0f));
	vec::Vec3 c2_top = vec3::VAdd(c2_pos, vec3::VGet(0.0f, c2_half, 0.0f));
	vec::Vec3 c2_bottom = vec3::VAdd(c2_pos, vec3::VGet(0.0f, -c2_half, 0.0f));

	float c1_r = StCas<float>(c1->GetCollisionR());
	float c2_r = StCas<float>(c2->GetCollisionR());

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

bool ModeGame::PlayerToMakimonoCollision(PlayerBase* player, at::vspc<Makimono>& makimono)
{
	if(!player || makimono.empty())
	{
		return false;
	}
	
	// プレイヤーのカプセル
	const vec::Vec3 pPos = player->GetPos();
	const float pHalf = player->GetColSubY();
	const float pR = StCas<float>(player->GetCollisionR());

	const vec::Vec3 pTop = vec3::VAdd(pPos, vec3::VGet(0.0f, pHalf, 0.0f));
	const vec::Vec3 pBottom = vec3::VAdd(pPos, vec3::VGet(0.0f, -pHalf, 0.0f));

	for(auto& sp : makimono)
	{
		Makimono* makimono = sp.get();
		if(!makimono)
		{
			continue;
		}
		if(!makimono->IsVisible())
		{
			continue;
		}
		// 巻物カプセル
		const vec::Vec3 mPos = makimono->GetPos();
		const float mHalf = makimono->GetColSubY();
		const float mR = StCas<float>(makimono->GetCollisionR());

		const vec::Vec3 mTop	= vec3::VAdd(mPos, vec3::VGet(0.0f, mHalf, 0.0f));
		const vec::Vec3 mBottom = vec3::VAdd(mPos, vec3::VGet(0.0f, -mHalf, 0.0f));
		
		//　カプセル同士の当たり判定
		if(DxlibConverter::HitCheckCapsuleToCapsule(pTop, pBottom, pR, mTop, mBottom, mR))
		{
			makimono->SetVisible(false); // 巻物を消す

			player->AddMakimono(1); // プレイヤーの巻物所持数を増やす

			return true;
		}
	}
	return false;
}

// キャラと宝箱の当たり判定処理
bool ModeGame::CharaToTreasureHitCollision(CharaBase* chara, const at::vspc<TreasureBase>& treasure)
{
	// 引数チェック
	if(!chara)
	{
		return false;
	}

	// 空中なら処理しない（設計に合わせて維持）
	if(!chara->GetLand())
	{
		return false;
	}

    // treasureが空なら当たらない
	if(treasure.empty())
	{
		return false;
	}

	// どれか一つでも当たればtrue
	bool hitAny = false;

	// コリジョン判定で引っかかった時に、escapeTbl[]順に角度を変えて回避を試みる
	float escapeTbl[] =
	{
		0, -10, 10, -20, 20, -30, 30, -40, 40, -50, 50, -60, 60, -70, 70, -80, 80,
	};

	// 宝箱ごとに判定
	for(const auto& treasure : treasure)
	{
		TreasureBase* t = treasure.get();
		if(!t) { continue; }

		// 角度を変えて回避を試みるループ
		for(int i = 0; i < static_cast<int>(sizeof(escapeTbl) / sizeof(escapeTbl[0])); i++)
		{
			// 移動前の位置を保存
			vec::Vec3 oldvPos = chara->GetPos();
			vec::Vec3 v = chara->GetInputVector();

			float rad = atan2((float)v.z, (float)v.x);
			float length = chara->GetMoveSpeed() * sqrt(v.z * v.z + v.x * v.x);
			float sx = _camera->GetPos().x - _camera->GetTarget().x;
			float sz = _camera->GetPos().z - _camera->GetTarget().z;
			float camrad = atan2(sz, sx);

			// escapeTbl[i] の分だけ移動量 v を回転
			float escape_rad = DEG2RAD(escapeTbl[i]);
			v.x = cos(rad + camrad + escape_rad) * length;
			v.z = sin(rad + camrad + escape_rad) * length;

			// v の分移動（試行）
			chara->SetPos(vec3::VAdd(chara->GetPos(), v));

			// 宝箱の指定フレームで判定
			const auto handleTreasure = t->GetHandle();
			const auto frameTreasure = t->GetHitCollisionFrame();

			vec::Vec3 hitPos;
			const bool hit = CollisionManager::GetInstance()->CheckPositionToMV1Collision(
				chara->GetPos(),
				handleTreasure,
				frameTreasure,
				chara->GetColSubY(),
				hitPos
			);

			if(hit)
			{
				// 位置を戻して「めり込み/加速」を防ぐ
				chara->SetPos(chara->GetOldPos());
				hitAny = true;
				break; // この宝箱の回避ループ終了
			}

			// 当たらなかった。元の座標に戻す
			chara->SetPos(oldvPos);
		}

		// 1つでも当たったら早期終了（必要なら継続でもOK）
		if(hitAny)
		{
			return true;
		}
	}

	return hitAny;
}
	
bool ModeGame::CharaToTreasureOpenCollision(PlayerBase* player, const at::vspc<TreasureBase>& treasures)
{
	// 引数チェック
	if (!player)
	{
		return false;
	}

	// 空中なら処理しない（設計に合わせて維持）
	if (!player->GetLand())
	{
		// 空中に行った場合は演出カメラを終了
		if (_isOpeningTreasure)
		{
			EndCinematicCamera();
			_isOpeningTreasure = false;
			_treasureHoldSec = 0.0f;
			auto sound = gGlobal._soundServer->Get("60");
			if (sound && sound->IsPlay())
			{
				sound->Stop();
			}
		}
		if (_treasureOpenUi)
		{
			_treasureOpenUi->SetVisible(false);
		}
		// 連打型宝箱もリセット
		for (const auto& sp : treasures)
		{
			if (auto rapidFire = dynamic_cast<TreasureRapidFire*>(sp.get()))
			{
				if (!rapidFire->IsOpen())
				{
					rapidFire->ResetButtonCount();
				}
			}
		}
		return false;
	}

	const int key = ApplicationMain::GetInstance()->GetKey();
	const int trg = ApplicationMain::GetInstance()->GetTrg();
	const int holdA = (key & PAD_INPUT_1) != 0;
	const bool pressedA = (trg & PAD_INPUT_1) != 0;

	bool inAnyTreasure = false; // どれか一つでも当たればtrue

	// 宝箱の指定フレームで判定
	for (const auto& sp : treasures)
	{
		TreasureBase* treasure = sp.get();
		if (!treasure)
		{
			continue;
		}

		// すでに開いているなら
		if (!treasure->IsVisible() || treasure->IsOpen())
		{
			continue;
		}
		const auto handleTreasure = treasure->GetHandle();
		const auto OpenCollision = treasure->GetOpenCollisionFrame();
		if (handleTreasure < 0 || OpenCollision < 0)
		{
			continue;
		}
		// 当たり判定
		vec::Vec3 hitPos;
		const bool inTreasure = CollisionManager::GetInstance()->CheckPositionToMV1Collision
		(
			player->GetPos(),
			handleTreasure,
			OpenCollision,
			player->GetColSubY(),
			hitPos
		);
		if (!inTreasure)
		{
			continue;
		}

		// 少なくともどれかの宝箱の範囲
		inAnyTreasure = true;

		// 連打型宝箱の処理
		if (auto rapidFire = dynamic_cast<TreasureRapidFire*>(treasure))
		{
			// Aボタンが押された時にカウント追加
			if (pressedA)
			{
				rapidFire->AddButtonCount();

				// SE再生（連打音）
				auto soundButton = gGlobal._soundServer->Get("60");
				if (soundButton)
				{
					soundButton->Play();
				}

				// 必要回数に達したら開く
				if (rapidFire->GetCurrentButtonCount() >= rapidFire->GetRequiredButtonCount())
				{
					_treasureTakenCount++;
					rapidFire->SetOpen(true);

					if (_counterUi)
					{
						_counterUi->DecreaseTreasureCount();
					}

					// エフェクト再生
					if (_doyaEffect)
					{
						_doyaEffect->SetTargetPlayer(player);
						_doyaEffect->PlayEffect(treasure->GetPos());
					}

					// 宝箱を開けた時の音波
					EnemySoundManager::GetInstance()->EmitSound(
						treasure->GetPos(),
						5,
						400.0f,
						10.0f
					);

					return true;
				}
			}
		}
		// 通常の長押し型宝箱の処理
		else
		{
			// Aボタンを押していなかったら開けない
			if (!holdA)
			{
				// Aボタンが離されたタイミングで演出カメラを終了
				if (_isOpeningTreasure)
				{
					EndCinematicCamera();
					_isOpeningTreasure = false;
					_treasureHoldSec = 0.0f;
					auto sound = gGlobal._soundServer->Get("60");
					if (sound && sound->IsPlay())
					{
						sound->Stop();
					}
				}
				_isOpeningTreasure = false;
				_treasureHoldSec = 0.0f;

				// TreasureOpenUiを表示（プレイヤーの位置で）
				if (_treasureOpenUi)
				{
					_treasureOpenUi->SetVisible(true);
					_treasureOpenUi->SetSize(100);
				}

				continue;
			}
			else
			{
				if (_treasureOpenUi)
				{
					_treasureOpenUi->SetVisible(false);
				}
				// 開け始めの1回だけ音波を発生
				if (!_isOpeningTreasure)
				{
					// 宝箱を開けた時の音波を発生
					EnemySoundManager::GetInstance()->EmitSound(
						treasure->GetPos(),
						5,
						400.0f,
						10.0f
					);
				}
			}

			// 開ける処理開始
			if (!_isOpeningTreasure)
			{
				TreasureOpeningCameraControl();
				_isOpeningTreasure = true;
				_treasureHoldSec = 0.0f;

				auto sound = gGlobal._soundServer->Get("60");
				if (sound && !sound->IsPlay())
				{
					sound->Play();
				}
			}

			// 経過時間を計算
			const float dt = 1.0f / 60.0f;
			_treasureHoldSec += dt;

			// 3秒間ホールドで取得
			if (_treasureHoldSec >= CHECK_OPEN_TIME)
			{
				_treasureTakenCount++;
				_treasureHoldSec = 0.0f;
				treasure->SetOpen(true);
				EndCinematicCamera();
				_isOpeningTreasure = false;

				// お宝のカウントを減らす
				if (_counterUi)
				{
					_counterUi->DecreaseTreasureCount();
				}

				if (_treasureOpenUi)
				{
					_treasureOpenUi->SetVisible(false);
				}

				// エフェクト再生
				if (_doyaEffect)
				{
					_doyaEffect->SetTargetPlayer(player);
					_doyaEffect->PlayEffect(treasure->GetPos());
				}

				// 宝箱を開けた時の音波を発生
				EnemySoundManager::GetInstance()->EmitSound(
					treasure->GetPos(),
					5,
					400.0f,
					10.0f
				);

				return true;
			}
		}
		break; // 1つの宝箱に対して処理するので、当たったらループを抜ける
	}
	// どの宝箱範囲にも入ってない or A押してない等ならリセット
	if (!inAnyTreasure)
	{
		// 宝箱範囲外に出た場合、演出カメラを終了
		if (_isOpeningTreasure)
		{
			EndCinematicCamera();
			_isOpeningTreasure = false;
			_treasureHoldSec = 0.0f;
			auto sound = gGlobal._soundServer->Get("60");
			if (sound && sound->IsPlay())
			{
				sound->Stop();
			}
		}

		if (_treasureOpenUi)
		{
			_treasureOpenUi->SetVisible(false);
		}

		// 連打型宝箱もリセット
		for (const auto& sp : treasures)
		{
			if (auto rapidFire = dynamic_cast<TreasureRapidFire*>(sp.get()))
			{
				if (!rapidFire->IsOpen())
				{
					rapidFire->ResetButtonCount();
				}
			}
		}
	}
	return false;
}

bool ModeGame::CharaToTreasureRapidFireCollision(PlayerBase* player, const at::vspc<TreasureRapidFire>& treasures)
{
	// TreasureRapidFire を TreasureBase として扱う
	at::vspc<TreasureBase> baseTreasures;
	baseTreasures.reserve(treasures.size());
	for (const auto& t : treasures)
	{
		baseTreasures.push_back(t);
	}

	return CharaToTreasureOpenCollision(player, baseTreasures);
}

// キャラ同士の押し出し処理
bool ModeGame::PushChara(CharaBase* move, CharaBase* stop)
{
	if(!move || !stop) { return false; }

	MapBase* _map = (_objectServer ? _objectServer->GetMap() : nullptr);

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

bool ModeGame::IsPlayerAttack(PlayerBase* player, at::vspc<EnemyBase>& enemy)
{
	_enemiesInAttackRange.clear();

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


	player = _player.get();

	float halfAngle = DEG2RAD(60.0f); // 60度
	float rad = 240.0f; // 半径240

	CollisionManager::GetInstance()->SetDebugDraw(_d_view_collision);

	bool anyhit = false;

	// 全ての敵に対して範囲チェック（攻撃ボタンが押されていなくても実行）
	for(auto& enemy : enemy)
	{
		if(!enemy->IsAlive())
		{
			continue;
		}

		if(enemy->GetIsInvincible())
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
			// 範囲内の敵をリストに追加（UI表示用）
			_enemiesInAttackRange.push_back(enemy.get());

			_attackUi->SetVisible(true);
			_attackUi->SetSize(100);
			_attackUi->Show(player->GetPos());

			// 攻撃ボタンが押された場合のみダメージ処理
			if(trg & PAD_INPUT_2)
			{
				anyhit = true;
				enemy->StartDamage();
				_showKnockdownMessage = true;
				_knockdownMessageSec = 1.0f; // 表示時間 1秒
			}
		}
	}

	// ヒットした時だけ攻撃アニメ開始＆ロックON
	if(anyhit)
	{
		_tanukiAttackAnimId = player->PlayAnimation("shippokougeki", false);
		auto soundAttack = gGlobal._soundServer->Get("10");
		if(soundAttack)
		{
			soundAttack->Play();
		}
		_isTanukiAttackPlaying = (_tanukiAttackAnimId != -1);
	}

	return false;
}

bool ModeGame::PlayerToGoalHitCollision(PlayerBase* player, Goal* goal)
{
	
	// 無効チェック
	if(!player || !goal)
	{
		return false;
	}
	// 空中なら処理しない（設計に合わせて維持）
	if(!player->GetLand())
	{
		return false;
	}
	// プレイヤーの座標
	auto playerPos = player->GetPos();
	auto playerColY = player->GetColSubY();
	// ゴールの指定フレームで判定
	const auto handleGoal = goal->GetModelHandle();
	const auto frameGoal  = goal->GetHitCollisionFrame();

	// プレイヤーと指定したコリジョンフレームで当たり判定
	vec::Vec3 hitPos;
	const bool hit = CollisionManager::GetInstance()->CheckPositionToMV1Collision
	(
		playerPos,
		handleGoal,
		frameGoal,
		playerColY,
		hitPos
	);

	return hit;
}

bool ModeGame::PlayerToTutorialCollision(PlayerBase* player, at::vspc<Tutorial> tutorial)
{
	if(!player || !_d_use_collision)
	{
		return false;
	}

	for(auto&& tutorial : tutorial)
	{
		if(!tutorial)
		{
			continue;
		}

		int collisionFrame = tutorial->GetTutorialCollisionFrame();
		if(collisionFrame < 0)
		{
			continue;
		}

		int handle = tutorial->GetHandle();
		if(handle < 0)
		{
			continue;
		}

		vec::Vec3 playerPos = player->GetPos();
		MATRIX modelMat = tutorial->MakeModelMatrix();
		MV1SetMatrix(handle, modelMat);
		MV1RefreshCollInfo(handle, collisionFrame);


		vec::Vec3 hitPos;
		if(CollisionManager::GetInstance()->CheckPositionToMV1Collision(
			playerPos,
			handle,
			collisionFrame,
			player->GetColSubY(),
			hitPos
		))
		{
			tutorial->PlayTutorial();
			return true;
		}
	}

	return false;
}