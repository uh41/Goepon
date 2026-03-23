#include "ModeGameOver.h"
#include "ApplicationMain.h"
#include "modegame.h"
#include "modegameoverload.h"
#include "appframe.h"

// 時間の進行と正規化
namespace
{
	constexpr float kDt = 1.0f / 60.0f; // 60FPS想定のフレーム時間（秒）

	float Lerp(float a, float b, float t)
	{
		return a + (b - a) * t;
	}

	float Clamp01(float v)
	{
		return std::clamp(v, 0.0f, 1.0f);
	}
}

bool ModeGameOver::Initialize()
{
	if(!base::Initialize()) return false;

	_debugCurrentStageId = "Stage1"; // �f�t�H���g
	if(_ownerGame)
	{
		auto* game = dynamic_cast<ModeGame*>(_ownerGame);
		if(game)
		{
			_debugCurrentStageId = game->GetCurrentStageId();
		}
	}

	int screenW = 0;
	int screenH = 0;
	GetScreenState(&screenW, &screenH, nullptr);

	_screenCx = screenW / 2;
	_screenCy = screenH / 2;

	_fadeSec = 0.0f;
	_spotAlpha = 0;
	_spotRadius = _spotStartRadius;
	_hasValidSpotCenter = false;

	UpdateSpotCenterFromPlayer();

	return true;
}

bool ModeGameOver::Terminate()
{
	// �����`��p�Ƀ��[�h�����O���t����
	if(_overlayHandle != -1)
	{
		DeleteGraph(_overlayHandle);
		_overlayHandle = -1;
		_showOverlayImmediate = false;
	}

	base::Terminate();
	return true;
}

bool ModeGameOver::Process()
{
	base::Process();
	ModeServer::GetInstance()->SkipProcessUnderLayer();

	// フェード進行（プレイヤー中心は毎フレーム追従）
	UpdateSpotCenterFromPlayer();

	_fadeSec += kDt;
	const float t = Clamp01(_fadeSec / _fadeDurationSec);

	// 半径を縮める（最初は広く、最後は0）
	_spotRadius = Lerp(_spotStartRadius, _spotEndRadius, t);

	// 暗さを増やす（0→255）
	_spotAlpha = StCas<int>(Lerp(0.0f, 255.0f, t));

	int trg = ApplicationMain::GetInstance()->GetTrg();
	if(trg & PAD_INPUT_1)
	{
		if(gGlobal._soundServer)
		{
			auto se = gGlobal._soundServer->Get("73");
			if(se)
			{
				se->Stop();
			}
		}

		SaveData sd{};
		if(SaveManager::TryLoad(sd, SaveManager::GetDefaultPath()))
		{
			ModeBase* existing = ModeServer::GetInstance()->Get("game");
			if(existing)
			{
				auto* game = dynamic_cast<ModeGame*>(existing);
				if(game)
				{
					game->ApplySaveData(sd);
					game->ResetEnemiesToInitialPositions();
					ModeServer::GetInstance()->Del(this);
					return true;
				}
			}

			auto* newGame = new ModeGame();
			newGame->SetInitialStageId(sd.stageId);
			ModeServer::GetInstance()->Add(newGame, 0, "game");
			ModeServer::GetInstance()->ProcessInit();

			{
				ModeBase* gm = ModeServer::GetInstance()->Get("game");
				if(gm)
				{
					auto* game = dynamic_cast<ModeGame*>(gm);
					if(game)
					{
						game->ApplySaveData(sd);
						game->ResetEnemiesToInitialPositions();
					}
				}
			}

			ModeServer::GetInstance()->Del(this);
			return true;
		}

		if(ModeServer::GetInstance()->Get("gameoverload") == nullptr)
		{
			ModeServer::GetInstance()->Add(new ModeGameOverLoad(nullptr, _debugCurrentStageId), 300, "gameoverload");
			ModeServer::GetInstance()->ProcessInit();
		}
		ModeServer::GetInstance()->Del(this);
		return true;
	}

	return true;
}

bool ModeGameOver::Render()
{
	base::Render();

	// 画面全体を暗くする
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 180); // 半透明の黒
	DrawBox(0, 0, 1920, 1080, GetColor(0, 0, 0), TRUE);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0); // ブレンドモードを元に戻す

	// 既存の黒背景UIは残しつつ、サンシャイン風暗転を上に載せる
	// ※ 文字は暗転の上に描く（読みやすさ優先）にする場合は順序を入れ替える
	DrawSpotlightFade();

	// UI枠（既存）
	DrawBox(BgLeft, BgTop, BgRight, BgBottom, GetColor(255, 255, 255), FALSE);

	SetFontSize(TitleFontSize);
	DrawString(TitlePosX, TitlePosY, GameOverMessage, GetColor(WhiteR, WhiteG, WhiteB));

	SetFontSize(HintFontSize);
	DrawString(HintPosX, HintPosY, HintMessage, GetColor(HintR, HintG, HintB));

	if(_showOverlayImmediate && _overlayHandle != -1)
	{
		int screenW, screenH;
		GetScreenState(&screenW, &screenH, nullptr);

		int imgW, imgH;
		GetGraphSize(_overlayHandle, &imgW, &imgH);

		int x = (screenW - imgW) / 2;
		int y = (screenH - imgH) / 2;

		DrawGraph(x, y, _overlayHandle, TRUE);
	}

	return true;
}

// プレイヤーの位置からスポットライトの中心を更新し、成功したかを返す
bool ModeGameOver::UpdateSpotCenterFromPlayer()
{
	_hasValidSpotCenter = false; // 毎回リセットして、成功したらtrueにする方式

	// オーナーゲームやプレイヤーが存在しない場合は失敗
	if(!_ownerGame)
	{
		return false;
	}

	// プレイヤーの位置を取得
	auto playerSp = _ownerGame->GetPlayerTanuki(); 
	if(!playerSp)
	{
		return false;
	}

	// プレイヤーのワールド座標にオフセットを加えてスポットライトの中心とし、スクリーン座標に変換する
	const vec::Vec3 world = vec3::VAdd(playerSp->GetPos(), vec3::VGet(0.0f, 60.0f, 0.0f));
	const VECTOR dxWorld = DxlibConverter::VecToDxLib(world);
	const VECTOR scr = ConvWorldPosToScreenPos(dxWorld);

	// 画面外/失敗対策（DXLibは失敗時の値が状況依存なので緩くチェック）
	if(std::isfinite(scr.x) && std::isfinite(scr.y))
	{
		_screenCx = StCas<int>(scr.x);
		_screenCy = StCas<int>(scr.y);
		_hasValidSpotCenter = true;
		return true;
	}

	return false;
}

void ModeGameOver::DrawSpotlightFade() const
{
	int screenW = 0;
	int screenH = 0;
	GetScreenState(&screenW, &screenH, nullptr);

	// まず全画面を黒くする
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, _spotAlpha);
	DrawBox(0, 0, screenW, screenH, GetColor(0, 0, 0), TRUE);

	// 中心座標
	const int cx = _screenCx;
	const int cy = _screenCy;

	const float r0 = (_spotRadius > 0.0f) ? _spotRadius : 0.0f;
	const float r1 = std::sqrt(StCas<float>(screenW * screenW + screenH * screenH));

	// 円形の穴を描くため、360度をいくつかの四角形に分割して描画する
	for(int deg = 0; deg < 360; deg += _spotRingStepDeg)
	{
		// 角度をラジアンに変換
		const float a0 = DEG2RAD(StCas<float>(deg));
		const float a1 = DEG2RAD(StCas<float>(deg + _spotRingStepDeg));

		// 内側の円と外側の円の4点を計算
		const int x0i = cx + StCas<int>(std::cos(a0) * r0);
		const int y0i = cy + StCas<int>(std::sin(a0) * r0);
		const int x0o = cx + StCas<int>(std::cos(a0) * r1);
		const int y0o = cy + StCas<int>(std::sin(a0) * r1);

		const int x1i = cx + StCas<int>(std::cos(a1) * r0);
		const int y1i = cy + StCas<int>(std::sin(a1) * r0);
		const int x1o = cx + StCas<int>(std::cos(a1) * r1);
		const int y1o = cy + StCas<int>(std::sin(a1) * r1);

		DrawQuadrangle
		(
			x0i, y0i,   // 内側: 角度a0
			x0o, y0o,   // 外側: 角度a0
			x1o, y1o,   // 外側: 角度a1
			x1i, y1i,   // 内側: 角度a1
			GetColor(0, 0, 0), TRUE
		);
	}

	// ブレンドモードを元に戻す
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}

