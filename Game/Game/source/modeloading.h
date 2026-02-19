#pragma once
#include "appframe.h"

class ModeLoading : public ModeBase
{
public:
	ModeLoading();
	virtual ~ModeLoading();
	virtual bool Initialize() override;
	virtual bool Terminate() override;
	virtual bool Process() override;
	virtual bool Render() override;

private:
	// UI関連
	int _handleLoadingBG;		// ローディング背景画像
	int _handleLoadingBar;		// ローディングバー画像
	int _handlePressButton;		// ボタン表示画像

	float _dotAnimTime;			// ドットアニメーション用タイマー
	int _dotCount;				// 表示するドット数

	bool _canProceed;			// ボタン入力可能フラグ
	float _buttonAlpha;			// ボタン点滅用アルファ値
};

