/*********************************************************************/
// * \file   TreasureRapidFire.h
// * \brief  連打型宝箱クラス
// *
// * \author 市村義春
// * \date   2026/03/07
// * \作業内容: 新規作成 市村義春　2026/03/07
//			 
/*********************************************************************/
#pragma once
#include "appframe.h"
#include "objectbase.h"
#include "TreasureBase.h"

class TreasureRapidFire : public TreasureBase
{
	typedef TreasureBase base;
public:
	bool Initialize() override; // 初期化
	bool Terminate()  override; // 終了
	bool Process()    override; // 処理
	bool Render()     override; // 描画

	// 連打カウント関連
	int GetMaxCount() const { return _maxCount; }	// 開けるのに必要な連打回数を取得
	int GetNowCount() const { return _nowCount; }	// 現在の連打回数を取得
	void AddCount();	// ボタンを1回押した時に呼ぶ
	void ResetCount();	// 範囲外に出た時などにリセット

	void RenderGaugeRF(const vec::Vec3& playerPos, float progress) override;

protected:
	int _attachIndex;

	// 連打型用のパラメータ
	int _maxCount;	// 開けるのに必要な連打回数
	int _nowCount;  // 現在の連打回数
	float _buttonResetTimer;  // 連打リセットタイマー（入力がない時間）
	static constexpr float BUTTON_RESET_TIME = 2.0f; // 入力がないとリセット

	// ゲージ描画用のパラメータ
	int _gaugeWidth;          // 長方形の幅
	int _gaugeHeight;         // 長方形の高さ
	int _gaugeBorderThickness; // 枠の太さ
	unsigned int _bgColor;    // 背景色
	unsigned int _fillColor;  // 塗りつぶし色
	unsigned int _borderColor; // 枠の色
	unsigned int _textColor;  // テキストの色

	// ゲージの内部描画処理
	void DrawRectGauge(int centerX, int centerY, float progress);
};
