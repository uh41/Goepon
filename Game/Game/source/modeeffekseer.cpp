#include "modeeffekseer.h"
#include "appframe.h"
#include "applicationmain.h"
#include "applicationglobal.h"

// 初期化
bool ModeEffekseer::Initialize()
{
	if(!base::Initialize()) { return false; }

	// エフェクトリソースを読み込む。
	// 読み込む時に大きさを指定する。
	_effect_resource_handle = LoadEffekseerEffect("res/Laser01.efkefc", 10.0f);

	return true;
}

// 終了
bool ModeEffekseer::Terminate()
{
	base::Terminate();

	// エフェクトリソースを削除する
	DeleteEffekseerEffect(_effect_resource_handle);

	return true;
}

// 計算処理
bool ModeEffekseer::Process()
{
	base::Process();
	int key = ApplicationMain::GetInstance()->GetKey();
	int trg = ApplicationMain::GetInstance()->GetTrg();

	// このモードより下のレイヤーはProcess()を呼ばない
	ModeServer::GetInstance()->SkipProcessUnderLayer();

	// ESCキーで閉じる
	bool close = false;
	if(trg & PAD_INPUT_9)
	{
		close = true;
	}

	// モードカウンタを使って60fpsでエフェクトを生成
	if(GetModeCount() % 60 == 0)
	{
		// エフェクトを再生する。
		_playing_effect_handle = PlayEffekseer3DEffect(_effect_resource_handle);

		// エフェクトの位置をリセットする。
		_position_x = 0.0f;
	}

	// 再生中のエフェクトを移動する。
	SetPosPlayingEffekseer3DEffect(_playing_effect_handle, _position_x, 50.f, 0);
	_position_x += 0.2f;

	// Effekseerにより再生中のエフェクトを更新する。
	UpdateEffekseer3D();

	// 閉じる
	if(close)
	{
		// このモードを削除する
		ModeServer::GetInstance()->Del(this);
	}

	return true;
}

// 描画処理
bool ModeEffekseer::Render()
{
	base::Render();

	// DXライブラリのカメラとEffekseerのカメラを同期する。
	Effekseer_Sync3DSetting();

	// Effekseerにより再生中のエフェクトを描画する。
	DrawEffekseer3D();

	return true;
}
