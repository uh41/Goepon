/*********************************************************************/
// * \file   treasure.h
// * \brief  お宝クラス
// *
// * \author 石森虹大
// * \date   2026/1/25
// * \作業内容: 新規作成 石森虹大 2026/1/25
//			 
/*********************************************************************/
#pragma once
#include "appframe.h"
#include "objectbase.h"
#include "TreasureBase.h"

class Treasure : public TreasureBase
{
	typedef TreasureBase base;
public:
	bool Initialize() override; // 初期化
	bool Terminate()  override; // 終了
	bool Process()    override; // 処理
	bool Render()     override; // 描画

	// ゲージ描画
	void RenderGauge(const vec::Vec3& playerPos);
protected:    
	int _attachIndex;    

	// ゲージ描画用のパラメータ（長方形ゲージ）
	void DrawRectGauge(int centerX, int centerY, float progress);

	int _gaugeWidth;          // ゲージの横幅
	int _gaugeHeight;         // ゲージの高さ
	int _gaugeBorderThickness; // 枠の太さ
	unsigned int _bgColor;    // 背景色
	unsigned int _fillColor;  // 塗りつぶし色
	unsigned int _borderColor; // 枠線の色
	unsigned int _textColor;  // テキストの色
};

