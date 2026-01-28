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

	CollisionManager::GetInstance()->SetDebugDraw(_d_view_collision);

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
		CollisionManager::GetInstance()->SetDebugDraw(_d_view_collision);
	}
	if(_d_view_collision)
	{
			MV1SetFrameVisible(_map->GetHandleMap(), _map->GetFrameMapCollision(), TRUE);
		
	}
	else
	{
			MV1SetFrameVisible(_map->GetHandleMap(), _map->GetFrameMapCollision(), FALSE);
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
		// 好みの色に変更可（R,G,B）
		CollisionManager::GetInstance()->RenderDebug(0, 255, 255);
	}

	return true;
}

