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

// デバックの初期化
bool ModeGame::DebugInitialize()
{
	// その他初期化
	_d_view_collision = true;
	_d_use_collision = false;
	_d_view_camera_info = true;
	_d_view_shadow_map = false;

	return true;
}

bool ModeGame::DebugProcess()
{
	int key = ApplicationMain::GetInstance()->GetKey();
	int trg = ApplicationMain::GetInstance()->GetTrg();
	// ESCキーでメニューを開く
	if(trg & PAD_INPUT_9)
	{
		ModeMenu* modemenu = new ModeMenu();
		// ModeGameより上のレイヤーにメニューを登録する
		ModeServer::GetInstance()->Add(modemenu, 99, "menu");
		// オーナーにこの ModeGame を設定
		modemenu->SetOwner(this);
		// ModeMenuにメニュー項目を追加する
		modemenu->Add(new MenuItemViewCollision(this, "ViewCollision"));
		modemenu->Add(new MenuItemUseCollision(this, "UseCollision"));
		modemenu->Add(new MenuItemViewCameraInfo(this, "ViewCameraInfo"));
		modemenu->Add(new MenuItemLaunchEffekseer(this, "Effekseer"));
		modemenu->Add(new MenuItemViewShadowMap(this, "ShadowMapView"));
		// カメラ操作モード切替項目を追加
		modemenu->Add(new MenuItemCameraControlMode(this, "CameraControlMode"));
	}

	//

	// デバッグ機能
	if(trg & PAD_INPUT_7)
	{
		_d_view_collision = !_d_view_collision;
	}
	if(_d_view_collision)
	{
			MV1SetFrameVisible(_map->GetHandleMap(), _map->GetFrameMapCollision(), TRUE);
	}
	else
	{
			MV1SetFrameVisible(_map->GetHandleMap(), _map->GetFrameMapCollision(), FALSE);
	}

	for(auto& t : _treasure)
	{
		if(!t) continue;
		int h = t->GetModelHandle();
		int hf = t->GetHitCollisionFrame();
		if(h >= 0 && hf >= 0)
		{
			MV1SetFrameVisible(h, hf, _d_view_collision ? TRUE : FALSE);
		}
	}

	return true;
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
			DrawFormatString(sx, y, GetColor(192, 255, 192), "Cam Pos : %0.2f, %0.2f, %0.2f", _camera->_vPos.x, _camera->_vPos.y, _camera->_vPos.z); y += line;
			DrawFormatString(sx, y, GetColor(192, 255, 192), "Cam Target : %0.2f, %0.2f, %0.2f", _camera->_vTarget.x, _camera->_vTarget.y, _camera->_vTarget.z); y += line;
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
				vec3::VAdd(_camera->_vTarget, vec3::VGet(-linelength, 0.0f, 0.0f)),
				vec3::VAdd(_camera->_vTarget, vec3::VGet(linelength, 0.0f, 0.0f)),
				GetColor(255, 255, 0));
			DxlibConverter::DrawLine3D(
				vec3::VAdd(_camera->_vTarget, vec3::VGet(0.0f, -linelength, 0.0f)),
				vec3::VAdd(_camera->_vTarget, vec3::VGet(0.0f, linelength, 0.0f)),
				GetColor(255, 255, 0));
			DxlibConverter::DrawLine3D(
				vec3::VAdd(_camera->_vTarget, vec3::VGet(0.0f, 0.0f, -linelength)),
				vec3::VAdd(_camera->_vTarget, vec3::VGet(0.0f, 0.0f, linelength)),
				GetColor(255, 255, 0));
		}
	}

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
				float rad            = static_cast<float>(p->GetCollisionR());
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


	// シャドウマップの表示
	if(_d_view_shadow_map)
	{
		TestDrawShadowMap(_map->GetHandleShadowMap(), 0, 0, 512, 512);
	}

	if(_d_view_camera_info)
	{
		_camera->Render();
	}

	
	return true;
}

