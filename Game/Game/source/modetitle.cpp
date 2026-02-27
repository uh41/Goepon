#include "modetitle.h"
#include "modeloading.h"

#ifdef _DEBUG
#include <crtdbg.h>
#define NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
#else
#define NEW new
#endif

ModeTitle::ModeTitle()
{
	Initialize();
}

ModeTitle::~ModeTitle()
{
	Terminate();
}

bool ModeTitle::Initialize()
{
	// 背景画像を読み込む（パスはプロジェクトのリソース配置に合わせて変更）
	_handle = LoadGraph("res/Title/BG_op.png");
	if(_handle == -1)
	{
		return false;
	}

	// 狸キャラデータを作成
	_player = std::make_shared<TitleTanuki>();
	if(_player)
	{
		_player->Initialize();
		// 固定位置に配置（Y座標を適切に設定）
		_player->SetPos(vec::Vec3(0.0f, 0.0f, 0.0f));
		_player->SetDir(vec::Vec3(0.0f, 0.0f, 1.0f));
	}

	// タイトル用のカメラを作成
	_cam = new TitleCamera();
	if(_cam)
	{
		_cam->Initialize();
	}

	// フェードなし - 即座に表示
	_state = ModeBase::State::WAIT;

	return true;
}

bool ModeTitle::Terminate()
{
	if(_handle != -1)
	{
		DeleteGraph(_handle);
		_handle = -1;
	}

	// 狸キャラ開放
	if(_player)
	{
		_player->Terminate();
		_player.reset();
	}

	// カメラ開放
	if(_cam)
	{
		delete _cam;
		_cam = nullptr;
	}

	return true;
}

bool ModeTitle::Process()
{
	// タイトルは最前面レイヤーとして画面の処理と描画スキップ
	ModeServer::GetInstance()->SkipProcessUnderLayer();
	ModeServer::GetInstance()->SkipRenderUnderLayer();

	int trg = ApplicationBase::GetInstance()->GetTrg();

	// カメラとプレイヤーの処理
	if(_cam) { _cam->Process(); }
	if(_player) { _player->Process(); }

	// ステート処理
	switch(_state)
	{
		case ModeBase::State::WAIT:
			// ボタンが押されるまで待機
			if(trg & PAD_INPUT_2)
			{
				_state = ModeBase::State::DONE;
			}
			break;
		case ModeBase::State::DONE:
			// 次のモードへ移行
			ModeServer::GetInstance()->Add(NEW ModeLoading(), 1, "loading");
			ModeServer::GetInstance()->Del(this);
			break;
	}

	return true;
}

bool ModeTitle::Render()
{
	if(!_cam)
	{
		return false;
	}

	base::Render();

	// 背景画像描画
	if(_handle != -1)
	{
		DrawGraph(0, 0, _handle, TRUE);
	}

	// カメラ設定更新
	SetCameraPositionAndTarget_UpVecY(DxlibConverter::VecToDxLib(_cam->GetPos()), DxlibConverter::VecToDxLib(_cam->GetTarget()));
	SetCameraNearFar(_cam->GetClipNear(), _cam->GetClipFar());

	// 視野角設定
	float fov_deg = 30.0f;
	float fov_rad = DEG2RAD(fov_deg);
	SetupCamera_Perspective(fov_rad);

	// 狸モデル描画
	if(_player)
	{
		_player->Render();
	}

	return true;
}