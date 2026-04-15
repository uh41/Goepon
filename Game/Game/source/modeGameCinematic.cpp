#include "modegame.h"
#include "playerform.h"

bool ModeGame::StartPlayerRotation()
{
	// ★★★ タヌキプレイヤーのみを対象に回転演出を初期化 ★★★
	PlayerBase* targetPlayer = PlayerFactory::GetTanukiPlayer();

	if(!targetPlayer)
	{
		return false; // タヌキプレイヤーが存在しない場合は処理しない
	}

	// 回転演出の初期化
	_isPlayerRotating = true;
	_playerRotationTimer = 0.0f;
	_playerRotationDuration = 2.0f; // 回転演出の総時間（秒）

	// 現在の回転角度を取得
	_playerInitialRotation = targetPlayer->GetRotationY();

	// 目標角度: dir.z = 1 の方向 = 0度（正面）
	_playerTargetRotation = 0.0f;

	// タヌキプレイヤーの操作を無効化
	targetPlayer->SetInputEnabled(false);

	return true;
}


// フェイドイン関係
void ModeGame::StartSpotLightFadeIn()
{
	int screenW = 0, screenH = 0;
	GetScreenState(&screenW, &screenH, nullptr);

	_spotFadeInCx = screenW / 2;
	_spotFadeInCy = screenH / 2;

	_spotFadeInSec    = 0.0f;
	_spotFadeInRadius = 0.0f;
	_spotFadeInActive = true;

	UpdateSpotLightCenterFromActivePlayer();

}

bool ModeGame::UpdateSpotLightCenterFromActivePlayer()
{
	PlayerBase* activePlayer = PlayerForm::GetInstance()->GetPlayer();
	if(!activePlayer)
	{
		return false; // プレイヤーが存在しない場合は処理しない
	}

	const vec::Vec3 world = vec3::VAdd(activePlayer->GetPos(), vec3::VGet(0.0f, 60.0f, 0.0f));
	const VECTOR dxWorld  = DxlibConverter::VecToDxLib(world);
	const VECTOR view	  = ConvWorldPosToScreenPos(dxWorld);

	// 画面外の座標はスポットライトの中心にしない
	if(std::isfinite(view.x) && std::isfinite(view.y))
	{
		_spotFadeInCx = StCas<int>(view.x);
		_spotFadeInCy = StCas<int>(view.y);
		return true;
	}
	
	return false;
}

void ModeGame::ProcessSpotlightFadeIn()
{
	if(!_spotFadeInActive)
	{
		return;
	}

	UpdateSpotLightCenterFromActivePlayer();

	_spotFadeInSec += 1.0f / 60.0f; // 60FPS想定
	const float t = std::clamp(_spotFadeInSec / _spotFadeInDurationSec, 0.0f, 1.0f);

	int screenW = 0;
	int screenH = 0;
	GetScreenState(&screenW, &screenH, nullptr);

	const float endRadius = std::sqrt(StCas<float>(screenW * screenW + screenH * screenH));
	_spotFadeInRadius = endRadius * t;

	if(t >= 1.0f)
	{
		_spotFadeInActive = false; // フェードイン完了
	}
}

void ModeGame::DrawSpotLightFadeIn() const
{
	if(!_spotFadeInActive)
	{
		return;
	}

	int screenW = 0;
	int screenH = 0;
	GetScreenState(&screenW, &screenH, nullptr);

	const int cx = _spotFadeInCx;
	const int cy = _spotFadeInCy;

	const float r0 = _spotFadeInRadius;
	const float r1 = std::sqrt(StCas<float>(screenW * screenW + screenH * screenH)); 

	// 外部だけ黒く塗る
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 255);

	for(int deg = 0; deg < 360; deg += SPOT_FADE_IN_STEP_DEG)
	{
		// 扇形の頂点を計算
		const float a0 = DEG2RAD(StCas<float>(deg));
		const float a1 = DEG2RAD(StCas<float>(deg + SPOT_FADE_IN_STEP_DEG));

		const int x0i = cx + StCas<int>(std::cos(a0) * r0);
		const int y0i = cy + StCas<int>(std::sin(a0) * r0);
		const int x0o = cx + StCas<int>(std::cos(a0) * r1);
		const int y0o = cy + StCas<int>(std::sin(a0) * r1);

		const int x1i = cx + StCas<int>(std::cos(a1) * r0);
		const int y1i = cy + StCas<int>(std::sin(a1) * r0);
		const int x1o = cx + StCas<int>(std::cos(a1) * r1);
		const int y1o = cy + StCas<int>(std::sin(a1) * r1);

		DrawQuadrangle(
			x0i, y0i,
			x0o, y0o,
			x1o, y1o,
			x1i, y1i,
			GetColor(0, 0, 0), TRUE
		);
	}

	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}