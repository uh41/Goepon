/*********************************************************************/
// * \file   treasure.cpp
// * \brief  お宝クラス
// *
// * \author 石森虹大
// * \date   2026/1/25
// * \作業内容: 新規作成 石森虹大 2026/1/25
//			 
/*********************************************************************/
#include "Treasure.h"

bool Treasure::Initialize()
{
	base::Initialize();


	LoadModel("res/Treasure/tuzura_05.mv1");
	if(_handle < 0) { DxLib::printfDx("Treasure model load failed\n"); return false; }


	_hitCollisionFrame = MV1SearchFrame(_handle, "Collision_04");
	_openCollisionFrame = MV1SearchFrame(_handle, "Collision_05");

	if(_hitCollisionFrame < 0) { DxLib::printfDx("HitFrame not found\n"); return false; }

	// if (_openCollisionFrame < 0) { DxLib::printfDx("OpenFrame not found\n"); }


	_attachIndex = -1;

	_objStatus = OBJSTATUS::NONE;

	_vPos = vec::Vec3{ 714.0f, 0.0f, 5803.0f };
	_vDir = vec::Vec3{ 0.0f, 0.0f, -1.0f };


	MV1SetupCollInfo(_handle, _hitCollisionFrame, 16, 16, 16);
	MV1SetFrameVisible(_handle, _hitCollisionFrame, FALSE);

	if(_openCollisionFrame >= 0)
	{
		MV1SetupCollInfo(_handle, _openCollisionFrame, 16, 16, 16);
		MV1SetFrameVisible(_handle, _openCollisionFrame, FALSE);
	}

	_isOpen    = false;
	_isVisible = true;

	// ゲージ描画用のパラメータ初期化（長方形）
	_gaugeWidth = 300;                      // 横幅
	_gaugeHeight = 30;                      // 高さ
	_gaugeBorderThickness = 3;              // 枠の太さ
	_bgColor = GetColor(50, 50, 50);        // 暗いグレー（背景）
	_fillColor = GetColor(255, 200, 0);     // 黄色（塗りつぶし）
	_borderColor = GetColor(200, 200, 200); // 明るいグレー（枠）
	_textColor = GetColor(255, 255, 255);   // 白（テキスト）

	ApplyMatrixAndRefreshCollInfo(_handle, _hitCollisionFrame, _openCollisionFrame, MakeModelMatrix());
	return true;
}


bool Treasure::Terminate()
{
	//_handle = -1;
	//_hitCollisionFrame = -1;
	//_openCollisionFrame = -1;
	//_isVisible = false;
	//_isOpen = false;
	//_attachIndex = -1;

	return true;
}

bool Treasure::Process()
{
	base::Process();

	ApplyMatrixAndRefreshCollInfo(_handle, _hitCollisionFrame, _openCollisionFrame, MakeModelMatrix());

	if(!_isOpen && _objStatus != OBJSTATUS::OPEN)
	{
		_objStatus = OBJSTATUS::OPEN;
	}
	return true;
}

bool Treasure::Render()
{
	base::Render();

	// 非表示なら描画しない（デバック)
	if (!_isVisible)
	{
		return true; 
	}


	if(_handle >= 0)
	{
		MV1DrawModel(_handle);
	}

	return true;
}

void Treasure::RenderGauge(const vec::Vec3& playerPos)
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

	// 長押し進行度を0.0～1.0として計算（仮で常に0.5とする）
	// ※実際の進行度は modegame.cpp から渡す必要があります
	float progress = 0.5f; // 仮の進行度

	// 長方形ゲージを描画（中心基準）
	DrawRectGauge(gaugeX, gaugeY, progress);
}

void Treasure::DrawRectGauge(int centerX, int centerY, float progress)
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

	// 進行度を％表示
	char text[32];
	sprintf_s(text, "%.0f%%", progress * 100.0f);

	const int textWidth = GetDrawStringWidth(text, static_cast<int>(strlen(text)));
	const int textX = gaugeX + (_gaugeWidth - textWidth) / 2; // ゲージ内で中央揃え
	const int textY = gaugeY + (_gaugeHeight - 16) / 2;       // 縦方向で中央揃え

	DrawString(textX, textY, text, _textColor);

	// 「Aボタン長押し！」のテキストをゲージの上に表示
	const char* instruction = "Aボタン長押し！";
	const int instrWidth = GetDrawStringWidth(instruction, static_cast<int>(strlen(instruction)));
	const int instrX = gaugeX + (_gaugeWidth - instrWidth) / 2; // ゲージ幅で中央揃え
	const int instrY = gaugeY - 25; // ゲージの上に配置

	DrawString(instrX, instrY, instruction, _textColor);
}