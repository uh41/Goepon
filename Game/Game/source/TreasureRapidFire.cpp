/*********************************************************************/
// * \file   TreasureRapidFire.cpp
// * \brief  連打型宝箱クラス
// *
// * \author 市村義春
// * \date   2026/03/07
// * \作業内容: 新規作成 市村義春　2026/03/07
//			 
/*********************************************************************/
#include "TreasureRapidFire.h"
#include "camera.h"
#include <cmath>

bool TreasureRapidFire::Initialize()
{
	base::Initialize();

	LoadModel("res/Treasure/tuzura_05.mv1");
	if (_handle < 0) { DxLib::printfDx("TreasureRapidFire model load failed\n"); return false; }

	_hitCollisionFrame = MV1SearchFrame(_handle, "Collision_04");
	_openCollisionFrame = MV1SearchFrame(_handle, "Collision_05");

	if (_hitCollisionFrame < 0) { DxLib::printfDx("HitFrame not found\n"); return false; }

	_attachIndex = -1;
	_objStatus = OBJSTATUS::NONE;

	_vPos = vec::Vec3{ 714.0f, 0.0f, 5803.0f };
	_vDir = vec::Vec3{ 0.0f, 0.0f, -1.0f };

	MV1SetupCollInfo(_handle, _hitCollisionFrame, 16, 16, 16);
	MV1SetFrameVisible(_handle, _hitCollisionFrame, FALSE);

	if (_openCollisionFrame >= 0)
	{
		MV1SetupCollInfo(_handle, _openCollisionFrame, 16, 16, 16);
		MV1SetFrameVisible(_handle, _openCollisionFrame, FALSE);
	}

	_isOpen = false;
	_isVisible = true;

	// 連打型のパラメータ初期化
	_requiredButtonCount = 5;
	_currentButtonCount = 0;
	_buttonResetTimer = 0.0f;

	// ゲージ描画用のパラメータ初期化（長方形）
	_gaugeWidth = 300;                      // 横幅
	_gaugeHeight = 30;                      // 高さ
	_gaugeBorderThickness = 3;              // 枠の太さ
	_bgColor = GetColor(50, 50, 50);        // 暗いグレー（背景）
	_fillColor = GetColor(255, 200, 0);     // 黄色（塗りつぶし）
	_borderColor = GetColor(200, 200, 200); // 明るいグレー（枠）
	_textColor = GetColor(255, 255, 255);   // 白（テキスト）

	// 初期状態で当たり判定情報を更新
	ApplyMatrixAndRefreshCollInfo(_handle, _hitCollisionFrame, _openCollisionFrame, MakeModelMatrix());
	return true;
}

bool TreasureRapidFire::Terminate()
{
	return true;
}

bool TreasureRapidFire::Process()
{
	base::Process();

	ApplyMatrixAndRefreshCollInfo(_handle, _hitCollisionFrame, _openCollisionFrame, MakeModelMatrix());

	// 連打リセットタイマー処理
	if (_currentButtonCount > 0 && !_isOpen)
	{
		const float dt = 1.0f / 60.0f; // 60FPS想定
		_buttonResetTimer += dt;

		// 一定時間入力がなければリセット
		if (_buttonResetTimer >= BUTTON_RESET_TIME)
		{
			ResetButtonCount();
		}
	}

	if (!_isOpen && _objStatus != OBJSTATUS::OPEN)
	{
		_objStatus = OBJSTATUS::OPEN;
	}

	return true;
}

bool TreasureRapidFire::Render()
{
	base::Render();

	// 非表示なら描画しない
	if (!_isVisible)
	{
		return true;
	}

	if (_handle >= 0)
	{
		MV1DrawModel(_handle);
	}

	return true;
}

void TreasureRapidFire::AddButtonCount()
{
	if (!_isOpen)
	{
		_currentButtonCount++;
		_buttonResetTimer = 0.0f; // タイマーリセット
	}
}

void TreasureRapidFire::ResetButtonCount()
{
	_currentButtonCount = 0;
	_buttonResetTimer = 0.0f;
}

void TreasureRapidFire::RenderGauge(const vec::Vec3& playerPos)
{
	// 宝箱が開いている場合は描画しない
	if (_isOpen)
	{
		return;
	}

	// 宝箱の中心位置の3D座標を2D座標に変換
	vec::Vec3 topPos = _vPos;
	topPos.y += 100.0f;  // 宝箱の上にオフセット（必要に応じて調整）

	VECTOR screenPos = ConvWorldPosToScreenPos(DxlibConverter::VecToDxLib(topPos));

	// 画面外チェック
	if (screenPos.z < 0.0f || screenPos.z > 1.0f)
	{
		return;
	}

	// ゲージの中心位置を宝箱の真上に設定
	int gaugeX = static_cast<int>(screenPos.x);
	int gaugeY = static_cast<int>(screenPos.y);

	// 進行度を計算
	float progress = static_cast<float>(_currentButtonCount) / static_cast<float>(_requiredButtonCount);
	if (progress > 1.0f) progress = 1.0f;

	// 長方形ゲージを描画（中心基準）
	DrawRectGauge(gaugeX, gaugeY, progress);
}

void TreasureRapidFire::DrawRectGauge(int centerX, int centerY, float progress)
{
	// ゲージの左上座標を計算（中心基準から左上にシフト）
	const int gaugeX = centerX - (_gaugeWidth / 2);  // 中心を基準に左にシフト
	const int gaugeY = centerY - _gaugeHeight - 30;  // ゲージを上に配置（+テキスト分のスペース）

	// 背景（暗いグレー）を描画
	DrawBox(
		gaugeX,
		gaugeY,
		gaugeX + _gaugeWidth,
		gaugeY + _gaugeHeight,
		_bgColor,
		TRUE
	);

	// 進行度に応じた塗りつぶし（黄色）を描画
	if (progress > 0.0f)
	{
		const int fillWidth = static_cast<int>(_gaugeWidth * progress);
		DrawBox(
			gaugeX,
			gaugeY,
			gaugeX + fillWidth,
			gaugeY + _gaugeHeight,
			_fillColor,
			TRUE
		);
	}

	// 枠線を描画
	DrawBox(
		gaugeX,
		gaugeY,
		gaugeX + _gaugeWidth,
		gaugeY + _gaugeHeight,
		_borderColor,
		FALSE
	);

	// 枠を太くするために複数回描画
	for (int i = 1; i < _gaugeBorderThickness; ++i)
	{
		DrawBox(
			gaugeX - i,
			gaugeY - i,
			gaugeX + _gaugeWidth + i,
			gaugeY + _gaugeHeight + i,
			_borderColor,
			FALSE
		);
	}

	// カウント数をゲージの中央に表示
	char text[32];
	sprintf_s(text, "%d/%d", _currentButtonCount, _requiredButtonCount);

	const int textWidth = GetDrawStringWidth(text, static_cast<int>(strlen(text)));
	const int textX = gaugeX + (_gaugeWidth - textWidth) / 2; // ゲージ内で中央揃え
	const int textY = gaugeY + (_gaugeHeight - 16) / 2;       // 縦方向で中央揃え

	DrawString(textX, textY, text, _textColor);

	// 「Aボタン連打！」のテキストをゲージの上に表示
	const char* instruction = "Aボタン連打！";
	const int instrWidth = GetDrawStringWidth(instruction, static_cast<int>(strlen(instruction)));
	const int instrX = gaugeX + (_gaugeWidth - instrWidth) / 2; // ゲージ幅で中央揃え
	const int instrY = gaugeY - 25; // ゲージの上に配置

	DrawString(instrX, instrY, instruction, _textColor);
}