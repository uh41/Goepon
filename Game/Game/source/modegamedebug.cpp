/*********************************************************************/
// * \file   modegamedebug.cpp
// * \brief  モードゲームクラス(デバック用)
// *
// * \author 鈴木裕稀
// * \date   2025/12/15
// * \作業内容: 新規作成 鈴木裕稀　2025/12/15
/*********************************************************************/

#include "modegame.h"
#include "menuitembase.h"

#ifdef _DEBUG
#include <crtdbg.h>
#define NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
#else
#define NEW new
#endif

// デバックの初期化
bool ModeGame::DebugInitialize()
{
	// その他初期化
	_d_view_collision = true;
	_d_use_collision = true;
	_d_view_camera_info = true;
	_d_view_shadow_map = false;

	//CollisionManager::GetInstance()->SetDebugDraw(_d_view_collision);

	return true;
}

bool ModeGame::DebugCameraControl()
{
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

	return false;
}

bool ModeGame::DebugProcess()
{
	int key = ApplicationMain::GetInstance()->GetKey();
	int trg = ApplicationMain::GetInstance()->GetTrg();
	// ESCキーでメニューを開く
	if(trg & PAD_INPUT_9)
	{
		ModeMenu* modemenu = NEW ModeMenu();
		// ModeGameより上のレイヤーにメニューを登録する
		ModeServer::GetInstance()->Add(modemenu, 99, "menu");
		// オーナーにこの ModeGame を設定
		modemenu->SetOwner(this);
		// ModeMenuにメニュー項目を追加する
		modemenu->Add(NEW MenuItemViewCollision(this, "ViewCollision"));
		modemenu->Add(NEW MenuItemUseCollision(this, "UseCollision"));
		modemenu->Add(NEW MenuItemViewCameraInfo(this, "ViewCameraInfo"));
		modemenu->Add(NEW MenuItemLaunchEffekseer(this, "Effekseer"));
		modemenu->Add(NEW MenuItemViewShadowMap(this, "ShadowMapView"));
		// カメラ操作モード切替項目を追加
		modemenu->Add(NEW MenuItemCameraControlMode(this, "CameraControlMode"));
	}

	//
	MapBase* _map = (_objectServer ? _objectServer->GetMap() : nullptr);

	// デバッグ機能
	if(trg & PAD_INPUT_7)
	{
		_d_view_collision = !_d_view_collision;
		CollisionManager::GetInstance()->SetDebugDraw(_d_view_collision);
	}
	// Map は null ガード（ここも落ちやすい）
	if(_map)
	{
		MV1SetFrameVisible(_map->GetHandleMap(), _map->GetFrameMapCollision(), _d_view_collision ? TRUE : FALSE);
	}

	// Treasure が vector になった前提：全要素を処理
	for(const auto& treasure : _treasure)
	{
		if(!treasure) continue;

		MV1SetFrameVisible(treasure->GetModelHandle(), treasure->GetHitCollisionFrame(), _d_view_collision ? TRUE : FALSE);
		MV1SetFrameVisible(treasure->GetModelHandle(), treasure->GetOpenCollisionFrame(), _d_view_collision ? TRUE : FALSE);
	}

	// デバック用タイマー（転ばせる）
	if(_showKnockdownMessage)
	{
		const float dt = 1.0f / 60.0f; // 60FPS想定
		_knockdownMessageSec -= dt;
		if(_knockdownMessageSec <= 0.0f)
		{
			_showKnockdownMessage = false;
			_knockdownMessageSec = 0.0f;
		}
	}

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
		_savedCamPos = _camera->GetPos();
		_savedCamTarget = _camera->GetTarget();
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
		_camera->SetPos(_savedCamPos);
		_camera->SetTarget(_savedCamTarget);
		_hasSavedCameraState = false;
		// カメラ操作モードを無効にする
		_bCameraControlMode = false;
	}
}

// デバック用の描画
bool ModeGame::DebugRender()
{
	// 画面にデバッグ文字列を出力（DrawFormatString を使用）
	{
		int sx = 500;
		int sy = 10;
		int line = 18;
		int y = sy;
		int modelHandle = -1;
		int frameIndex = -1;
		
		MapBase* _map = (_objectServer ? _objectServer->GetMap() : nullptr);
		if(_map && _map->GetHandleMap() > 0)
		{
			modelHandle = _map->GetHandleMap();
		}
		if(_map)
		{
			frameIndex = _map->GetFrameMapCollision();
		}

		DrawFormatString(sx, y, GetColor(255, 255, 255), "Map Handle(main) : %d", modelHandle); y += line;
		DrawFormatString(sx, y, GetColor(255, 255, 255), "Map Frame(collision) : %d", frameIndex); y += line;
		DrawFormatString(sx, y, GetColor(255, 255, 0), "ViewCollision : %d   UseCollision : %d", _d_view_collision ? 1 : 0, _d_use_collision ? 1 : 0); y += line;
		DrawFormatString(sx, y, GetColor(255, 255, 0), "ViewShadowMap : %d", _d_view_shadow_map ? 1 : 0); y += line;

		if(_camera)
		{
			DrawFormatString(sx, y, GetColor(192, 255, 192), "Cam Pos : %0.2f, %0.2f, %0.2f", _camera->GetPos().x, _camera->GetPos().y, _camera->GetPos().z); y += line;
			DrawFormatString(sx, y, GetColor(192, 255, 192), "Cam Target : %0.2f, %0.2f, %0.2f", _camera->GetTarget().x, _camera->GetTarget().y, _camera->GetTarget().z); y += line;
		}
	}

	// 0,0,0を中心に線を引く
	{
		float linelenght = 1000.f;
		VECTOR v = { 0, 0, 0 };
		DrawLine3D(VAdd(v, VGet(-linelenght, 0, 0)), VAdd(v, VGet(linelenght, 0, 0)), GetColor(255, 0, 0));
		DrawLine3D(VAdd(v, VGet(0, -linelenght, 0)), VAdd(v, VGet(0, linelenght, 0)), GetColor(0, 255, 0));
		DrawLine3D(VAdd(v, VGet(0, 0, -linelenght)), VAdd(v, VGet(0, 0, linelenght)), GetColor(0, 0, 255));
	}

	// カメラターゲットを中心に短い線を引く
	{
		float linelength = 10.f;
		if(_camera)
		{
			DxlibConverter::DrawLine3D(
				vec3::VAdd(_camera->GetTarget(), vec3::VGet(-linelength, 0.0f, 0.0f)),
				vec3::VAdd(_camera->GetTarget(), vec3::VGet(linelength, 0.0f, 0.0f)),
				GetColor(255, 255, 0));
			DxlibConverter::DrawLine3D(
				vec3::VAdd(_camera->GetTarget(), vec3::VGet(0.0f, -linelength, 0.0f)),
				vec3::VAdd(_camera->GetTarget(), vec3::VGet(0.0f, linelength, 0.0f)),
				GetColor(255, 255, 0));
			DxlibConverter::DrawLine3D(
				vec3::VAdd(_camera->GetTarget(), vec3::VGet(0.0f, 0.0f, -linelength)),
				vec3::VAdd(_camera->GetTarget(), vec3::VGet(0.0f, 0.0f, linelength)),
				GetColor(255, 255, 0));
		}
	}

	//// markerDGR範囲の描画（customIdごとにグループ化して描画）
	//if (_d_view_collision)
	//{
	//	std::string path = "res/map/";
	//	std::string jsonFile = "markerDGR.json";
	//	std::string jsonObjectName = "stage";

	//	std::ifstream ifs(path + jsonFile);
	//	if (ifs.is_open())
	//	{
	//		nlohmann::json jsonData;

	//		ifs >> jsonData;

	//		// JSONが正しく読み込めたかデバッグ表示
	//		DrawFormatString(10, 200, GetColor(255, 255, 0), "JSON loaded success");

	//		if (jsonData.contains(jsonObjectName))
	//		{
	//			nlohmann::json stage = jsonData.at(jsonObjectName);

	//			// customIdごとのmarkerDGRポイントをグループ化
	//			std::unordered_map<std::string, std::vector<vec::Vec3>> rangeGroups;

	//			for (auto& object : stage)
	//			{
	//				if (!object.contains("objectName")) continue;

	//				const std::string& name = object.at("objectName");

	//				if (name == "S_MarkerDGR")
	//				{
	//					vec::Vec3 pos;

	//					// 座標の取得（エラーハンドリング付き）
	//					if (object.contains("translate"))
	//					{
	//						auto& trans = object.at("translate");
	//						if (trans.contains("x")) trans.at("x").get_to(pos.x);
	//						if (trans.contains("y")) trans.at("y").get_to(pos.z);
	//						if (trans.contains("z")) trans.at("z").get_to(pos.y);
	//						pos.z *= -1.0f;

	//						std::string gid = "";
	//						if (object.contains("customId"))
	//						{
	//							object.at("customId").get_to(gid);
	//						}
	//						rangeGroups[gid].push_back(pos);
	//					}
	//				}
	//			}

	//			// デバッグ表示：検出されたグループ数
	//			DrawFormatString(10, 220, GetColor(255, 255, 0),
	//				"Range groups found: %d", static_cast<int>(rangeGroups.size()));

	//			// 各グループの範囲を描画
	//			unsigned int colors[] = {
	//				GetColor(255, 0, 0),    // 赤
	//				GetColor(0, 255, 0),    // 緑
	//				GetColor(0, 0, 255),    // 青
	//				GetColor(255, 255, 0),  // 黄
	//				GetColor(255, 0, 255),  // マゼンタ
	//				GetColor(0, 255, 255),  // シアン
	//			};
	//			int colorIndex = 0;
	//			int groupIdx = 0;

	//			for (const auto& [customId, points] : rangeGroups)
	//			{
	//				DrawFormatString(10, 240 + groupIdx * 20, GetColor(255, 255, 0),
	//					"Group[%d] ID:'%s' Points:%d",
	//					groupIdx, customId.c_str(), static_cast<int>(points.size()));

	//				if (points.size() < 2)
	//				{
	//					groupIdx++;
	//					continue;
	//				}

	//				unsigned int color = colors[colorIndex % 6];
	//				colorIndex++;

	//				const float heightOffset = 50.0f; // 床から十分浮かせる

	//				// 隣り合うポイント間に線を描画
	//				for (size_t i = 0; i < points.size(); ++i)
	//				{
	//					size_t nextIdx = (i + 1) % points.size();

	//					vec::Vec3 p1 = vec3::VAdd(points[i], vec3::VGet(0.0f, heightOffset, 0.0f));
	//					vec::Vec3 p2 = vec3::VAdd(points[nextIdx], vec3::VGet(0.0f, heightOffset, 0.0f));

	//					// DxLib形式に変換して描画
	//					VECTOR v1 = DxlibConverter::VecToDxLib(p1);
	//					VECTOR v2 = DxlibConverter::VecToDxLib(p2);
	//					DrawLine3D(v1, v2, color);

	//					// マーカー点も描画（太い線で）
	//					const float markerSize = 20.0f;
	//					vec::Vec3 marker1 = vec3::VAdd(p1, vec3::VGet(-markerSize, 0.0f, 0.0f));
	//					vec::Vec3 marker2 = vec3::VAdd(p1, vec3::VGet(markerSize, 0.0f, 0.0f));
	//					vec::Vec3 marker3 = vec3::VAdd(p1, vec3::VGet(0.0f, 0.0f, -markerSize));
	//					vec::Vec3 marker4 = vec3::VAdd(p1, vec3::VGet(0.0f, 0.0f, markerSize));

	//					VECTOR m1 = DxlibConverter::VecToDxLib(marker1);
	//					VECTOR m2 = DxlibConverter::VecToDxLib(marker2);
	//					VECTOR m3 = DxlibConverter::VecToDxLib(marker3);
	//					VECTOR m4 = DxlibConverter::VecToDxLib(marker4);

	//					DrawLine3D(m1, m2, color);
	//					DrawLine3D(m3, m4, color);

	//					// 縦線も追加（見やすくするため）
	//					vec::Vec3 markerDown = vec3::VAdd(p1, vec3::VGet(0.0f, -20.0f, 0.0f));
	//					VECTOR md = DxlibConverter::VecToDxLib(markerDown);
	//					DrawLine3D(v1, md, color);
	//				}
	//				groupIdx++;
	//			}
	//		}
	//	}
	//}

	// タヌキプレイヤーのカプセル当たり判定を表示
	if(_bShowTanuki && _d_view_collision)
	{
		// タヌキプレイヤーの参照を取得
		if(_playerTanuki)
		{
			PlayerBase* p = _playerTanuki.get();
			if(p && p->IsAlive())
			{
				// プレイヤーの現在位置とカプセルパラメータを再計算（CharaToTreasureHitCollision と同じ式）
				vec::Vec3 currentPos = p->GetPos();
				float rad            = StCas<float>(p->GetCollisionR());
				float half           = p->GetColSubY();

				// カプセルの上下端を計算
				vec::Vec3 capTop    = vec3::VAdd(currentPos, vec3::VGet(0.0f, half, 0.0f));
				vec::Vec3 capBottom = vec3::VAdd(currentPos, vec3::VGet(0.0f, -half, 0.0f));

				// DxLib の描画用に変換
				VECTOR top    = DxlibConverter::VecToDxLib(capTop);
				VECTOR bottom = DxlibConverter::VecToDxLib(capBottom);

				int color = GetColor(0, 255, 255); // シアン

				// カプセルの描画
				const int divNum = 16; // 分割数（見た目の滑らかさ）
				DrawCapsule3D(top, bottom, rad, divNum, color, color, TRUE);
			}
		}
	}

	MapBase* _map = (_objectServer ? _objectServer->GetMap() : nullptr);
	// シャドウマップの表示
	if(_d_view_shadow_map)
	{
		TestDrawShadowMap(_map->GetHandleShadowMap(), 0, 0, 512, 512);
	}

	if(_d_view_camera_info)
	{
		_camera->Render();
	}

	if(_d_view_collision)
	{
		// CollisionManager 側のデバッグ描画（線やマーカー）
		CollisionManager::GetInstance()->SetDebugDraw(true);
		CollisionManager::GetInstance()->RenderDebug(0, 255, 255);
	}
	else
	{
		CollisionManager::GetInstance()->SetDebugDraw(false);
	}

	// 各敵のセンサーを個別に描画
	for (auto& enemy : _enemyBase)
	{
		if (enemy->IsAlive() && enemy->GetEnemySensor())
		{
			enemy->GetEnemySensor()->RenderDetectionUI();
		}
	}

	// 宝箱を開けているメッセージ表示
	if(_isOpeningTreasure)
	{
		/*auto _playerPosx = _bShowTanuki ? _playerTanuki->GetPos().x : _player->GetPos().x;
		auto _playerPosz = _bShowTanuki ? _playerTanuki->GetPos().z : _player->GetPos().z;*/
		const char* msg = "お宝を開けています...(Aを押し続けてください)";
		int color = GetColor(255, 0, 0); // 黄色
		// 座標は適宜調整（ここでは画面左上(50, 400)に仮配置）
		DrawString(900, 500, msg, color);
	}

	// 敵を転ばせたメッセージ表示
	if(_showKnockdownMessage)
	{
		const char* msg = "敵を転ばせた";
		DrawString(900, 500, msg, GetColor(255, 255, 255));
	}

	if(anyDetected)
	{
		const char* alertMsg = "敵に発見された！変身できない！";
		// 座標は適宜調整（ここでは画面中央上部に仮配置）
		DrawString(600, 500, alertMsg, GetColor(255, 0, 0));
	}

	// --- ここに変身時間表示を追加 ---
	if(_changeTimeActive)
	{
		// 点滅制御がある場合は点滅フラグが true のときだけ表示
		if(_changeBlinkVisible)
		{
			// フォントサイズ / 描画位置
			int fontSize = 28;
			SetFontSize(fontSize);

			// 5秒以下で注意色、それ以外は白
			unsigned int color = (_changeTimeLimit <= 5.0f) ? GetColor(255, 64, 64) : GetColor(255, 255, 255);

			// 表示位置（左上に余白を確保）
			int x = 20;
			int y = 20;

			// 60秒以上なら MM:SS 表示、未満は秒（小数）表示
			if(_changeTimeLimit >= 60.0f)
			{
				int minutes = static_cast<int>(_changeTimeLimit) / 60;
				int seconds = static_cast<int>(_changeTimeLimit) % 60;
				DrawFormatString(x, y, color, "変身残り: %d:%02d", minutes, seconds);
			}
			else
			{
				DrawFormatString(x, y, color, "変身残り: %.1f s", _changeTimeLimit);
			}
		}
	}

	// 敵の聴覚範囲の描画
	if (auto* soundMgr = EnemySoundManager::GetInstance())
	{
		soundMgr->RenderDebug();
		soundMgr->RenderDebugEnemyHearing(_enemyBase);
	}

	return true;
}

