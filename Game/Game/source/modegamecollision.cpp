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

	// PlayerTanukiかどうかをチェック
	PlayerTanuki* tanuki = dynamic_cast<PlayerTanuki*>(chara);

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
		vec::Vec3 bodyHitPos; // 体の当たり位置
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
					bodyHitPos)) // 体の当たり位置を取得
				{
					hit = true;
					break;
				}
			}
		}

		if(hit)
		{
			// 体が当たった場合、タヌキなら頭の判定もチェック
			if(tanuki)
			{
				// 先に頭の判定をチェック（移動後の位置で）
				vec::Vec3 headHitPos; // 頭の当たり位置（別途用意）
				bool headHit = CheckTanukiHeadCollision(tanuki, obj, headHitPos);

				if(!headHit)
				{
					// 頭が当たっていない＝天井や壁にめり込もうとしている
					// 元の位置に戻す（押し出し処理）
					chara->SetPos(oldvPos);
					// 次の角度を試すためにループを続行
					continue;
				}

				// 頭の当たり判定のみを使用（タヌキの場合）
				// headHitPosを使ってY座標のみ調整
				vec::Vec3 tmpPos = chara->GetPos();
				tmpPos.y = headHitPos.y; // 頭の当たり位置のY座標を使用
				chara->SetPos(tmpPos);

				// Y変化を移動ベクトルに反映
				triedV.y += tmpPos.y - oldvPos.y;
			}
			else
			{
				// タヌキ以外は通常の地面移動処理（体の当たり判定を使用）
				vec::Vec3 tmpPos = chara->GetPos();
				tmpPos.y = bodyHitPos.y;
				chara->SetPos(tmpPos);
				triedV.y += chara->GetPos().y - oldvPos.y;
			}

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

bool ModeGame::CheckTanukiHeadCollision(PlayerTanuki* player, ObjectBase* obj, vec::Vec3& outHitPos)
{
	// プレイヤーが無効なら処理しない
	if(!player)
	{
		return false;
	}

	// マップの情報を取得
	MapBase* map = dynamic_cast<MapBase*>(obj);
	if(!map)
	{
		return false;
	}

	// 入力情報を取得
	int key = ApplicationMain::GetInstance()->GetKey();

	// 頭の位置を計算（入力方向に応じて変更）
	vec::Vec3 headPos = player->GetPos();
	vec::Vec3 offset = vec3::VGet(0.0f, 0.0f, 0.0f);
	float offsetScale = player->GetColSubY() * 1.5f;

	// 各方向の入力をチェックしてオフセットを累積
	bool hasInput = false;

	if(key & PAD_INPUT_UP)
	{
		offset.z += offsetScale; // 上方向
		hasInput = true;
	}
	if(key & PAD_INPUT_DOWN)
	{
		offset.z -= offsetScale; // 下方向
		hasInput = true;
	}
	if(key & PAD_INPUT_RIGHT)
	{
		offset.x += offsetScale; // 右方向
		hasInput = true;
	}
	if(key & PAD_INPUT_LEFT)
	{
		offset.x -= offsetScale; // 左方向
		hasInput = true;
	}

	// 斜め入力の場合、ベクトルを正規化して距離を保つ
	if(hasInput)
	{
		float length = sqrt(offset.x * offset.x + offset.z * offset.z);
		if(length > 0.0f)
		{
			offset.x = (offset.x / length) * offsetScale;
			offset.z = (offset.z / length) * offsetScale;
		}
		headPos = vec3::VAdd(headPos, offset);
	}
	else
	{
		// 入力がない場合はデフォルトで上方向（元の処理）
		headPos.z += offsetScale;
	}

	// 頭の位置で当たり判定をチェック
	bool headHit = false;

	auto& list = map->GetBlockPosList();
	for(size_t bi = 0; bi < list.size(); ++bi)
	{
		auto& block = list[bi];

		if(block.modelHandle >= 0)
		{
			// transform を適用
			MV1SetPosition(block.modelHandle, VGet(block.x, block.y, block.z));
			MV1SetRotationXYZ(block.modelHandle, VGet(block.rx, block.ry, block.rz));
			MV1SetScale(block.modelHandle, VGet(block.sx, block.sy, block.sz));
			MV1RefreshCollInfo(block.modelHandle, block.collisionFrame);
		}

		// 必要な条件を満たさない場合はスキップ
		if(block.modelHandle < 0) continue;
		if(block.collisionFrame < 0) continue;

		// 頭の位置で当たり判定（体より小さい半径を使用）
		if(CollisionManager::GetInstance()->CheckPositionToMV1Collision(
			headPos,
			block.modelHandle,
			block.collisionFrame,
			player->GetColSubY() * 0.3f, // 体の30%の半径（頭のサイズ）
			outHitPos)) // 頭の当たり位置を出力パラメータに格納
		{
			headHit = true;
			break;
		}
	}

	return headHit;
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

			if(_makimonoGetEffect)
			{
				_makimonoGetEffect->SetTargetPlayer(player);
				_makimonoGetEffect->PlayEffect(player->GetPos());
			}

			auto makimonoSe = gGlobal._soundServer->Get("70");
			if(makimonoSe)
			{
				makimonoSe->Play();
			}

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

	if(dynamic_cast<PlayerMono*>(player) != nullptr)
	{
		// PlayerMonoは宝箱を開けられない
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
					rapidFire->ResetCount();
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
	TreasureBase* currentTreasure = nullptr; // 現在処理中の宝箱

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
		currentTreasure = treasure;

		// 連打型宝箱の処理
		if (auto rapidFire = dynamic_cast<TreasureRapidFire*>(treasure))
		{
			// Aボタンが押された時にカウント追加
			if (pressedA)
			{
				rapidFire->AddCount();
				//player->SetInputEnabled(false);
				if(player)
				{
					player->PlayAnimation("takarabako_open", true);
				}

				// SE再生（連打音）
				auto soundButton = gGlobal._soundServer->Get("60");
				if (soundButton)
				{
					soundButton->Play();
				}

				// 宝箱を開けた時の音波
				EnemySoundManager::GetInstance()->EmitSound(
					treasure->GetPos(),
					5,
					400.0f,
					10.0f
				);

				// 必要回数に達したら開く
				if (rapidFire->GetNowCount() >= rapidFire->GetMaxCount())
				{
					_treasureTakenCount++;
					rapidFire->SetOpen(true);

					if(_goal && _treasureTakenCount >= _treasureRequiredCount)
					{
						_goal->SetCollisionEnabled(true);
					}

					if(player)
					{
						player->PlayAnimation("idle", true);
					}

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

					if(_TreasureOpenEffect)
					{
						_TreasureOpenEffect->PlayEffect(treasure->GetPos());
					}

					// 宝箱を開けた時の音波
					EnemySoundManager::GetInstance()->EmitSound(
						treasure->GetPos(),
						5,
						400.0f,
						10.0f
					);

					if (gGlobal._soundServer)
					{
						auto sOpen = gGlobal._soundServer->Get("4"); // "4" = tanuki_Tresure_open
						if (sOpen) sOpen->Play();
					}

					auto sound = gGlobal._soundServer->Get("60");
					if (sound && sound->IsPlay())
					{
						sound->Stop();
					}

					return true;
				}
			}
			/*else 
			{
				player->SetInputEnabled(true);
			}*/
			// 連打型は複数同時に処理可能なのでcontinue
			continue;
		}
		// 通常の長押し型宝箱の処理
		else
		{
			// 長押し型は1つずつしか開けられないので、最初に見つかった宝箱で処理を行う
			break;
		}
	}

	// 長押し型宝箱の処理（currentTreasureが長押し型の場合のみ実行）
	if (currentTreasure && !dynamic_cast<TreasureRapidFire*>(currentTreasure))
	{
		// 開けている宝箱が変わった場合はリセット
		if (_currentOpeningTreasure != currentTreasure)
		{
			//// 前の宝箱が開け途中だった場合は進行度をリセット
			//if (_currentOpeningTreasure != nullptr)
			//{
			//	_treasureProgressMap[_currentOpeningTreasure] = 0.0f;
			//}

			// 新しい宝箱に切り替え
			_currentOpeningTreasure = currentTreasure;
			_isOpeningTreasure = false;
			_treasureHoldSec = _treasureProgressMap[currentTreasure] * CHECK_OPEN_TIME;
			//_treasureHoldSec = 0.0f;
		}

		// Aボタンを押していなかったら開けない
		if (!holdA)
		{
			// Aボタンが離されたタイミングで演出カメラを終了
			if (_isOpeningTreasure)
			{
				EndCinematicCamera();
				_isOpeningTreasure = false;
				//player->SetInputEnabled(true);
				if(player)
				{
					player->PlayAnimation("idle", true);
				}
				auto sound = gGlobal._soundServer->Get("60");
				if (sound && sound->IsPlay())
				{
					sound->Stop();
				}
			}
			_isOpeningTreasure = false;
			//_treasureHoldSec = 0.0f;

			// TreasureOpenUiを表示（プレイヤーの位置で）
			if (_treasureOpenUi)
			{
				_treasureOpenUi->SetVisible(true);
				_treasureOpenUi->SetSize(100);
			}
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
					currentTreasure->GetPos(),
					5,
					400.0f,
					10.0f
				);
			}

			// 開ける処理開始
			if (!_isOpeningTreasure)
			{
				TreasureOpeningCameraControl();
				_isOpeningTreasure = true;
				//player->SetInputEnabled(false);
				if(player)
				{
					player->PlayAnimation("takarabako_open", true);
				}

				auto sound = gGlobal._soundServer->Get("60");
				if (sound && !sound->IsPlay())
				{
					sound->Play();
				}
			}

			// 経過時間を計算
			const float dt = 1.0f / 60.0f;
			_treasureHoldSec += dt;

			// この宝箱専用の進行度を計算してマップに保存
			float currentProgress = _treasureHoldSec / CHECK_OPEN_TIME;
			if (currentProgress > 1.0f) currentProgress = 1.0f;
			_treasureProgressMap[currentTreasure] = currentProgress;

			// 3秒間ホールドで取得
			if (_treasureHoldSec >= CHECK_OPEN_TIME)
			{
				_treasureTakenCount++;
				//_treasureHoldSec = 0.0f;
				currentTreasure->SetOpen(true);
				EndCinematicCamera();
				_isOpeningTreasure = false;

				if(_goal && _treasureTakenCount >= _treasureRequiredCount)
				{
					_goal->SetCollisionEnabled(true);
				}

				if(player)
				{
					player->PlayAnimation("idle", true);
				}

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
					_doyaEffect->PlayEffect(currentTreasure->GetPos());
				}

				if (_TreasureOpenEffect)
				{
					_TreasureOpenEffect->PlayEffect(currentTreasure->GetPos());
				}

				if(gGlobal._soundServer)
				{
					auto sOpen = gGlobal._soundServer->Get("4"); // "4" = tanuki_Tresure_open
					if(sOpen) sOpen->Play();
				}

				auto sound = gGlobal._soundServer->Get("60");
				if(sound && sound->IsPlay())
				{
					sound->Stop();
				}

				// 宝箱を開けた時の音波を発生
				EnemySoundManager::GetInstance()->EmitSound(
					currentTreasure->GetPos(),
					5,
					400.0f,
					10.0f
				);

				return true;
			}
		}
	}

	// どの宝箱範囲にも入ってない or A押してない等ならリセット
	if (!inAnyTreasure)
	{
		// 宝箱範囲外に出た場合、演出カメラを終了
		if (_isOpeningTreasure)
		{
			EndCinematicCamera();
			_isOpeningTreasure = false;
			//_treasureHoldSec = 0.0f;
			//// 開けていた宝箱の進行度をリセット
			//if (_currentOpeningTreasure != nullptr)
			//{
			//	_treasureProgressMap[_currentOpeningTreasure] = 0.0f;
			//}
			_currentOpeningTreasure = nullptr;
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

		//progress = 0.0f;	//進行度をリセット

		// 連打型宝箱もリセット
		for (const auto& sp : treasures)
		{
			if (auto rapidFire = dynamic_cast<TreasureRapidFire*>(sp.get()))
			{
				if (!rapidFire->IsOpen())
				{
					//rapidFire->ResetCount();
					auto sound = gGlobal._soundServer->Get("60");
					if (sound && sound->IsPlay())
					{
						sound->Stop();
					}
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

	vec::Vec3 actualDelta = vec3::VSub(move->GetPos(), oldpos);

	// move が現在表示されているプレイヤーインスタンスのいずれかであればカメラを移動
	bool isDisplayedPlayer = (move == _player.get() || move == _playerTanuki.get() || move == _playerMono.get());

	// 演出カメラ使用中は追従させない
	if(isDisplayedPlayer && _camera && !_useCinematicCamera)
	{
		_camera->MoveBy(actualDelta);
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

	if(player == nullptr)
	{
		return false;
	}

	player = _player.get();

	// たぬき状態なら攻撃UIを消して処理を抜ける（UIが残らないようにする）
	if(dynamic_cast<PlayerTanuki*>(player) != nullptr)
	{
		if(_attackUi)
		{
			_attackUi->SetVisible(false);
		}
		return false;
	}

	// 判定開始前にUIをリセット（前フレームの表示が残らないように）
	if(_attackUi)
	{
		_attackUi->SetVisible(false);
	}

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

		if(dynamic_cast<EnemyDog*>(enemy.get()) != nullptr)
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

			// 人間状態のみUIを表示（上でたぬきは弾いているのでここでは不要だが二重チェック）
			if(_attackUi && dynamic_cast<PlayerTanuki*>(player) == nullptr)
			{
				_attackUi->SetVisible(true);
				_attackUi->SetSize(100);
				_attackUi->Show(player->GetPos());
			}

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

	// 範囲内の敵が一人も居なければUIを隠す（念のため）
	if(_enemiesInAttackRange.empty() && _attackUi)
	{
		_attackUi->SetVisible(false);
	}

	// ヒットした時だけ攻撃アニメ開始＆ロックON
	if(anyhit)
	{
		if(_changeTimeActive)
		{
			// 既に変身中なら、残り時間が5秒以下なら即戻す（また、二度目の攻撃で戻す）
			if(_changeTimeLimit <= timelimit::START_TIME_LIMIT)
			{
				// UI やフラグ操作は Request 関数に任せる
				RequestReturnToTanukiFromHuman();
				return false;
			}
			else
			{
				// 変身中で残り時間が5秒より多い場合、攻撃で残り時間を5秒にする（点滅開始）
				_changeTimeLimit = timelimit::START_TIME_LIMIT;
				_changeBlinkTimer = 0.0f;
				_changeBlinkVisible = true;
			}
		}
		else
		{
			// 変身していなければ、新たに「攻撃による5秒点滅」を開始する
			_changeTimeActive = true;
			_changeTimeLimit = timelimit::START_TIME_LIMIT;
			_changeBlinkTimer = 0.0f;
			_changeBlinkVisible = true;
		}

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

	if(!goal->IsCollisionEnabled())
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

bool ModeGame::PlayerToSavePointCollision(PlayerBase* player)
{
	if(_savePoint.empty())
	{
		return false;
	}

	PlayerBase* checkPlayer;
	if(player)
	{
		checkPlayer = player;
	}
	else
	{
		checkPlayer = _player.get();
	}

	if(!checkPlayer)
	{
		return false;
	}

	for(auto& sp : _savePoint)
	{
		auto* savePoint = sp.get();
		if(!savePoint)
		{
			continue;
		}

		int h = savePoint->GetHandle();
		int f = savePoint->GetSavePointCollisionFrame();
		if(h < 0 || f < 0)
		{
			continue;
		}

		MATRIX model = savePoint->MakeModelMatrix();
		MV1SetMatrix(h, model);
		MV1RefreshCollInfo(h, f);

		vec::Vec3 hitPos;
		if(CollisionManager::GetInstance()->CheckPositionToMV1Collision(
			checkPlayer->GetPos(),
			h,
			f,
			checkPlayer->GetColSubY(),
			hitPos
		))
		{
			if(_lastSavedPoint != savePoint)
			{
				SavePlayer(checkPlayer);
				_lastSavedPoint = savePoint;

				// 先に「一回だけ抑制」フラグを処理する（ゲーム開始直後の誤再生対策）
				if(_suppressNextSavePointSound)
				{
					_suppressNextSavePointSound = false;
				}
				else
				{
					// 抑制フラグが立っていて、かつ同じセーブポイントなら効果音を再生しない
					if(!(_suppressSavePoint && _suppressedSavePoint == savePoint))
					{
						auto save = gGlobal._soundServer->Get("71");
						if(save)
						{
							save->Play();
						}
					}
				}
			}
			return true;
		}
	}

	// どのセーブポイントにも触れていない場合は、再度同じポイントでセーブできるようリセット
	_lastSavedPoint = nullptr;
	return false;
}