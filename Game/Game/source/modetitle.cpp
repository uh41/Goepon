#include "modetitle.h"
#include "modeloading.h"

#ifdef _DEBUG
#include <crtdbg.h>
#define NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
#else
#define NEW new
#endif

// ModeBase / Fade は appframe.h 経由で参照可能な前提
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
	_handle = LoadGraph(img::title);
	if(_handle == -1)
	{
		// 読み込み失敗時は false を返して呼び出し元で判定できるようにする
		return false;
	}

	// タイトル用のカメラを作成
	_cam = new TitleCamera();
	if(_cam)
	{
		_cam->Initialize();
		// カメラの位置やターゲットを必要に応じて調整
		_cam->SetPos(vec3::VGet(0.0f, 100.0f, -200.0f)); // カメラ位置
		_cam->SetTarget(_player->GetPos()); // 注視点
		_cam->SetClipNear(1.0f);
	}

	// タヌキモデルを作成
	_player = std::make_shared<TitleTanuki>();
	if(_player)
	{
		_player->Initialize();
		// 表示位置を調整
		_player->SetPos(vec3::VGet(0.0f, 0.0f, 0.0f));
		// モデルが正面を向くように向きの初期化も可能
		_player->SetDir(vec3::VGet(0.0f, 0.0f, 1.0f));
	}
	// フェードの初期化（黒→透過でフェードイン）
	Fade::GetInstance()->ColorMask(0, 0, 0, 255);
	Fade::GetInstance()->FadeIn(FADE_FRAME);

	_state = ModeBase::State::FADE_IN;
	_fadeTimer = 0;

	return true;
}

bool ModeTitle::Terminate()
{
	if(_handle != -1)
	{
		DeleteGraph(_handle);
		_handle = -1;
	}

	// タヌキ開放
	if (_player)
	{
		_player->Terminate();
		_player.reset();
	}

	// カメラ開放
	if (_cam)
	{
		delete _cam;
		_cam = nullptr;
	}

	return true;
}

bool ModeTitle::Process()
{
	// フェード進行
	Fade::GetInstance()->Process();

	// タイトルは最上位レイヤーとして下位の処理と描画をスキップ
	ModeServer::GetInstance()->SkipProcessUnderLayer();
	ModeServer::GetInstance()->SkipRenderUnderLayer();

	int trg = ApplicationBase::GetInstance()->GetTrg();

	if (_cam) { _cam->Process(); }
	if (_player) { _player->Process(); }

	switch(_state)
	{
	case ModeBase::State::FADE_IN:
		if(Fade::GetInstance()->IsFade() == false)
		{
			_state = ModeBase::State::WAIT;
		}
		break;
	case ModeBase::State::WAIT:
		if(trg & PAD_INPUT_2)
		{
			_state = ModeBase::State::FADE_OUT;
			Fade::GetInstance()->FadeOut(0, 0, 0, FADE_FRAME);
		}
		break;
	case ModeBase::State::FADE_OUT:
		if(Fade::GetInstance()->IsFade() == false)
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

	if (_handle != -1)
	{
		DrawGraph(0, 0, _handle, TRUE);
	}

	// カメラ設定更新
	SetCameraPositionAndTarget_UpVecY(DxlibConverter::VecToDxLib(_cam->GetPos()), DxlibConverter::VecToDxLib(_cam->GetTarget()));
	SetCameraNearFar(_cam->GetClipNear(), _cam->GetClipFar());


	// 視野角設定（角度は必要に応じて調整）
	float fov_deg = 30.0f;
	float fov_rad = DEG2RAD(fov_deg);
	SetupCamera_Perspective(fov_rad);

	// タヌキモデル描画
	if (_player)
	{
		_player->Render();
	}

	// フェード描画（上書き）
	Fade::GetInstance()->Render();

	return true;
}