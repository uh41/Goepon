#include "modetitle.h"
//#include "modeloading.h"
#include "mymath.h"
#include "modeopscenario.h"

#ifdef _DEBUG
#include <crtdbg.h>
#define NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
#else
#define NEW new
#endif

ModeTitle::~ModeTitle()
{
	Terminate();
}

bool ModeTitle::Initialize()
{
	// 読み込み関連
	_bgHandle    = LoadGraph(img::Title_GB);
	_titleHandle = LoadGraph(img::Title_Logo);

	// メニューのハンドルを読み込む
	_startYesHandle = LoadGraph(ui::Title_gamestart);
	_startNoHandle  = LoadGraph(ui::Title_gamestart_no);
	_exitYesHandle  = LoadGraph(ui::Title_gamefinish);
	_exitNoHandle   = LoadGraph(ui::Title_gamefinish_no);

	// ふすまのハンドルを読み込む
	_fusumaRighetHandle = LoadGraph(img::Fusuma_R);
	_fusumaLeftHandle   = LoadGraph(img::Fusuma_L);

	// 読み込み失敗していれば初期化失敗
	if(_bgHandle == -1)
	{
		return false;
	}
	if(_startYesHandle == -1 || _startNoHandle == -1 || _exitYesHandle == -1 || _exitNoHandle == -1)
	{
		return false;
	}

	_menu = MenuItem::Start; // デフォルトは「スタート」
	// ロゴ落下への初期化
	_titleX = 70.0f;
	_titleW = 0;
	_titleH = 0;
	_titleVY = 0.0f;
	_titleTargetY = 100.0f;
	_titleLanding = false;

	// タイトルロゴのサイズを取得
	if(_titleHandle != -1)
	{
		GetGraphSize(_titleHandle, &_titleW, &_titleH);
	}

	// ふすまサイズ取得&初期位置の設定
	if(_fusumaLeftHandle != -1)
	{
		GetGraphSize(_fusumaLeftHandle, &_fusumaW, &_fusumaH);
	}

	_fusumaY = 0.0f;

	// 初期状態は空いている
	_fusumaLeftX  = -StCas<float>(_fusumaW);	
	_fusumaRightX =  StCas<float>(_fusumaW);
	_fusumaState = FusumaState::None;
	_fusumaCnt = 0.0f;

	// 画面外上から開始
	_titleY = -StCas<float>(_titleH);

	// タヌキ生成
	_player = std::make_shared<TitleTanuki>();
	if(_player)
	{
		if(!_player->Initialize())
		{
			_player.reset();
			return false;
		}
		_player->SetPos(vec::Vec3(70.0f, -70.0f, 0.0f));
		_player->SetDir(vec::Vec3(1.0f, 0.0f, 0.0f));
	}

	// カメラ生成
	_cam = new TitleCamera();
	if(_cam)
	{
		_cam->Initialize();
	}

	_state = ModeBase::State::WAIT;

	if(!_soundServer && gGlobal._soundServer)
	{
		_soundServer = gGlobal._soundServer;
	}

	if(_soundServer)
	{
		_soundServer->Add("102", std::make_shared<soundserver::SoundItemBGM>(mp3::Title));
		auto bgm = _soundServer->Get("102");
		if(bgm)
		{
			bgm->Play(); // 追加後に明示的に再生
		}
	}

	return true;
}

bool ModeTitle::Terminate()
{
	if(_bgHandle != -1)
	{
		DeleteGraph(_bgHandle);
		_bgHandle = -1;
	}

	if(_titleHandle != -1)
	{
		DeleteGraph(_titleHandle);
		_titleHandle = -1;
	}

	if(_startYesHandle != -1)
	{
		DeleteGraph(_startYesHandle);
		_startYesHandle = -1;
	}
	if(_startNoHandle != -1)
	{
		DeleteGraph(_startNoHandle);
		_startNoHandle = -1;
	}
	if(_exitYesHandle != -1)
	{
		DeleteGraph(_exitYesHandle);
		_exitYesHandle = -1;
	}
	if(_exitNoHandle != -1)
	{
		DeleteGraph(_exitNoHandle);
		_exitNoHandle = -1;
	}

	if(_player)
	{
		_player->Terminate();
		_player.reset();
	}

	if(_cam)
	{
		_cam->Terminate();
		delete _cam;
		_cam = nullptr;
	}

	if(gGlobal._soundServer)
	{
		auto bgm = gGlobal._soundServer->Get("102");
		if(bgm)
		{
			bgm->Stop();
			gGlobal._soundServer->Del("102");
		}
	}

	if(_soundServer)
	{
		_soundServer->StopType(soundserver::SoundItemBase::TYPE::SE);
	}

	return true;
}

bool ModeTitle::Process()
{
	if(!_cam || !_player)
	{
		return false;
	}

	ModeServer::GetInstance()->SkipProcessUnderLayer();
	ModeServer::GetInstance()->SkipRenderUnderLayer();

	int trg = ApplicationBase::GetInstance()->GetTrg();

	// オブジェクト生成
	if(_cam) { _cam->Process(); }
	if(_player) { _player->Process(); }

	if(_titleHandle != -1 && !_titleLanding)
	{
		const float gravity = 1.2f;  // 重力加速度
		const float maxVY   = 40.0f; // 落下速度の上限
		const float bounce  = 0.55f; // 反発係数

		_titleVY += gravity; // 重力を加算
		if(_titleVY > maxVY)
		{
			_titleVY = maxVY; // 落下速度の上限を適用
		}

		_titleY += _titleVY; // 位置を更新

		// 着地判定
		if(_titleY >= _titleTargetY)
		{
			_titleY = _titleTargetY; // 位置を修正

			// バウンド
			if(std::abs(_titleVY) > 6.0f && bounce > 0.0f)
			{
				_titleVY = -_titleVY * bounce;
				_titleY += _titleVY; // 1フレーム分だけ戻す
			}
			else
			{
				_titleVY = 0.0f;
				_titleLanding = true;
			}
		}
	}

	// 選択移動
	MenuItem prevMenu = _menu;
	if(trg & PAD_INPUT_LEFT)
	{
		_menu = MenuItem::Start;
	}
	else if(trg & PAD_INPUT_RIGHT)
	{
		_menu = MenuItem::Exit;
	}

	if(prevMenu != _menu)
	{
		if(_soundServer)
		{
			_soundServer->StopType(soundserver::SoundItemBase::TYPE::SE);
			auto se = std::make_shared<soundserver::SoundItemSE>(mp3::UI_Henshin_pon);
			_soundServer->Add("se_title", se);
			se->Play();
		}
	}

	// ふすま閉じの更新
	const bool fusumaClosedNow = ProcessFusumaClose();

	switch (_state)
	{
	case ModeBase::State::WAIT:
		if (trg & PAD_INPUT_1)
		{
			if (_menu == MenuItem::Start)
			{
				// まずふすまを閉じる
				StartFusumaClose();
			}
			else
			{
				ModeServer::GetInstance()->Del(this);
				if (ApplicationBase::GetInstance())
				{
					ApplicationBase::GetInstance()->RequestExit();
				}
			}
		}

		// ふすまが閉じきったら遷移
		if (_fusumaState == FusumaState::closed)
		{
			_fusumaClosedWaitCnt++;	

			// 閉じた状態で少し待つ
			if(_fusumaClosedWaitCnt >= _fusumaClosedWaitFrames)
			{
				_state = ModeBase::State::DONE;
			}
		}
		break;

	case ModeBase::State::DONE:
		ModeServer::GetInstance()->Add(NEW ModeOpScenario(), 1, "opscenario");
		ModeServer::GetInstance()->ProcessInit();
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

	if(_bgHandle != -1)
	{
		DrawGraph(0, 0, _bgHandle, TRUE);
	}

	SetCameraPositionAndTarget_UpVecY(DxlibConverter::VecToDxLib(_cam->GetPos()), DxlibConverter::VecToDxLib(_cam->GetTarget()));
	SetCameraNearFar(_cam->GetClipNear(), _cam->GetClipFar());

	// 視野角の設定 
	float fov_deg = 30.0f;
	float fov_rad = DEG2RAD(fov_deg);
	SetupCamera_Perspective(fov_rad);

	if(_player)
	{
		_player->Render();
	}

	// タイトルロゴの描画
	if (_titleHandle != -1)
	{
		DrawGraph(StCas<int>(_titleX), StCas<int>(_titleY), _titleHandle, TRUE);
	}
	
	// メニューの描画
	int startW = 0, startH = 0;
	GetGraphSize(_startYesHandle, &startW, &startH);

	const int x = ui::MENU_UI_X;
	const int y = ui::MENU_UI_Y;

	const int spacing = ui::MENU_ITEM_SPACING;	
	if(_menu == MenuItem::Start)
	{
		DrawGraph(x, y, _startYesHandle, TRUE);
		DrawGraph(x + spacing, y, _exitNoHandle, TRUE);
	}
	else
	{
		DrawGraph(x, y, _startNoHandle, TRUE);
		DrawGraph(x + spacing, y, _exitYesHandle, TRUE);
	}

	// ふすまの描画
	RenderFusuma();
	return true;
}

void ModeTitle::StartFusumaClose()
{
	// すでに閉じていたら何もしない
	if(_fusumaState == FusumaState::closing || _fusumaState == FusumaState::closed)
	{
		return; 
	}

	_fusumaState = FusumaState::closing;
	_fusumaCnt = 0.0f; 
	_fusumaClosedWaitCnt = 0;
	// ふすま開始位置
	_fusumaLeftX = -StCas<float>(_fusumaW);
	_fusumaRightX = 1920.0f;
}

bool ModeTitle::ProcessFusumaClose()
{
	// 演出中ではなかったら
	if(_fusumaState != FusumaState::closing)
	{
		return false; 
	}

	_fusumaCnt += 1.0f; // フレームカウンタを進める	
	if(_fusumaCnt > _fusumaFrames)
	{
		_fusumaCnt = _fusumaFrames; 
	}

	const float screenW = 1920.0f;	
	
	const float leftStartX = -StCas<float>(_fusumaW);
	const float leftEndX = 0.0f;

	const float rightStartX = screenW;
	const float rightEndX = screenW - StCas<float>(_fusumaW);

	// イージング関数で位置を計算
	_fusumaLeftX = mymath::EasingOutQuad(_fusumaCnt, leftStartX, leftEndX, _fusumaFrames);
	_fusumaRightX = mymath::EasingOutQuad(_fusumaCnt, rightStartX, rightEndX, _fusumaFrames);

	// 閉じ終わりの判定
	if (_fusumaCnt >= _fusumaFrames)
	{
		_fusumaState = FusumaState::closed;
		return true; // 閉じ終わり
	}

	return false; // 閉じ途中
}

void ModeTitle::RenderFusuma() const
{
	if (_fusumaLeftHandle == -1 || _fusumaRighetHandle == -1)
	{
		return;
	}

	// Noneでも描画したいなら条件を外す
	if (_fusumaState == FusumaState::None)
	{
		return;
	}

	DrawGraph(StCas<int>(_fusumaLeftX), StCas<int>(_fusumaY), _fusumaLeftHandle, TRUE);
	DrawGraph(StCas<int>(_fusumaRightX), StCas<int>(_fusumaY), _fusumaRighetHandle, TRUE);
}