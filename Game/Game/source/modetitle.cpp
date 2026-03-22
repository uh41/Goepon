#include "modetitle.h"
#include "modeloading.h"

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
	// 読み込み
	_bgHandle    = LoadGraph("res/Title/Title_GB.png");
	_titleHandle = LoadGraph("res/Title/Title_Logo.png");

	// 読み込み失敗していれば初期化失敗
	if(_bgHandle == -1)
	{
		return false;
	}

	_startHandle = LoadGraph(ui::B_gamestart);

	// ロゴ落下への初期化
	_titleX = 30.0f;
	_titleW = 0;
	_titleH = 0;
	_titleVY = 0.0f;
	_titleTargetY = -130.0f;
	_titleLanding = false;

	// タイトルロゴのサイズを取得
	if(_titleHandle != -1)
	{
		GetGraphSize(_titleHandle, &_titleW, &_titleH);
	}

	// 画面外上から開始
	_titleY = -StCas<float>(_titleH);

	// �K�L�����f�[�^��쐬
	_player = std::make_shared<TitleTanuki>();
	if(_player)
	{
		if(!_player->Initialize())
		{
			_player.reset();
			return false;
		}

		// �Œ�ʒu�ɔz�u�iY���W��K�؂ɐݒ�j
		_player->SetPos(vec::Vec3(70.0f, -70.0f, 0.0f));
		_player->SetDir(vec::Vec3(1.0f, 0.0f, 0.0f));
	}

	// �^�C�g���p�̃J������쐬
	_cam = new TitleCamera();
	if(_cam)
	{
		_cam->Initialize();
	}

	// �t�F�[�h�Ȃ� - �����ɕ\��
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

	// �K�L�����J��
	if(_player)
	{
		_player->Terminate();
		_player.reset();
	}

	// �J�����J��
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

	return true;
}

bool ModeTitle::Process()
{
	if(!_cam || !_player)
	{
		return false;
	}

	// �^�C�g���͍őO�ʃ��C���[�Ƃ��ĉ�ʂ̏����ƕ`��X�L�b�v
	ModeServer::GetInstance()->SkipProcessUnderLayer();
	ModeServer::GetInstance()->SkipRenderUnderLayer();

	int trg = ApplicationBase::GetInstance()->GetTrg();

	// �J�����ƃv���C���[�̏���
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
	// �X�e�[�g����
	switch(_state)
	{
		case ModeBase::State::WAIT:
			// �{�^�����������܂őҋ@
			if(trg & PAD_INPUT_2)
			{
				_state = ModeBase::State::DONE;
			}
			break;
		case ModeBase::State::DONE:
			// ���̃��[�h�ֈڍs
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

	// �w�i�摜�`��
	if(_bgHandle != -1)
	{
		DrawGraph(0, 0, _bgHandle, TRUE);
	}

	DrawGraph(ui::START_CONFIG_X, ui::START_CONFIG_Y, _startHandle, TRUE);

	// �J�����ݒ�X�V
	SetCameraPositionAndTarget_UpVecY(DxlibConverter::VecToDxLib(_cam->GetPos()), DxlibConverter::VecToDxLib(_cam->GetTarget()));
	SetCameraNearFar(_cam->GetClipNear(), _cam->GetClipFar());

	// ����p�ݒ�
	float fov_deg = 30.0f;
	float fov_rad = DEG2RAD(fov_deg);
	SetupCamera_Perspective(fov_rad);

	// �K���f���`��
	if(_player)
	{
		_player->Render();
	}

	// タイトルロゴの描画
	if (_titleHandle != -1)
	{
		DrawGraph(StCas<int>(_titleX), StCas<int>(_titleY), _titleHandle, TRUE);
	}

	return true;
}