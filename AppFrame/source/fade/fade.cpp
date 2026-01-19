#include "fade.h"
#include "../mymath.h"
#include "../Application/applicationbase.h"

Fade::Fade()
{
	Initialize();
}

bool Fade::Initialize()
{
	_iColorR = 0;	// 赤
	_iColorG = 0;	// 緑
	_iColorB = 0;	// 青
	_iAlpha = 0;	// アルファ値

	// フェードイン用
	_iFadeStR = 0;	// フェードインの開始赤
	_iFadeStG = 0;	// フェードインの開始緑
	_iFadeStB = 0;	// フェードインの開始青
	_iFadeStA = 0;	// フェードインの開始アルファ

	// フェードアウト用
	_iFadeEdR = 0;	// フェードアウトの終了赤
	_iFadeEdG = 0;	// フェードアウトの終了緑
	_iFadeEdB = 0;	// フェードアウトの終了青
	_iFadeEdA = 0;	// フェードアウトの終了アルファ

	_iFadeFrames = 0;	// フェードのフレーム数
	_iFadeCnt = 0;		// フェードのカウント
	return true;
}

bool Fade::Terminate()
{
	return true;
}

Fade* Fade::GetInstance()
{
	static Fade instance;
	return &instance;
}

void Fade::ColorMask(int R, int G, int B, int alpha)
{
	_iColorR = R;
	_iColorG = G;
	_iColorB = B;
	_iAlpha = alpha;
}

// 現在のカラーマスクからフェードイン
void Fade::FadeIn(int frame)
{
	_iFadeStR = _iColorR;	// フェードインの開始赤
	_iFadeStG = _iColorG;	// フェードインの開始緑
	_iFadeStB = _iColorB;	// フェードインの開始青
	_iFadeStA = _iAlpha;	// フェードインの開始アルファ

	_iFadeEdR = _iColorR;	// フェードアウトの終了赤
	_iFadeEdG = _iColorG;	// フェードアウトの終了緑
	_iFadeEdB = _iColorB;	// フェードアウトの終了青
	_iFadeEdA = 0;			// フェードアウトの終了アルファ

	_iFadeFrames = frame;	// フェードのフレーム数
	_iFadeCnt = 0;			// フェードのカウント
}

// 指定の色にフェードアウト
void Fade::FadeOut(int R, int G, int B, int frame)
{
	_iFadeStR = _iColorR;	// フェードインの開始赤
	_iFadeStG = _iColorG;	// フェードインの開始緑
	_iFadeStB = _iColorB;	// フェードインの開始青
	_iFadeStA = _iAlpha;	// フェードインの開始アルファ

	_iFadeEdR = R;			// フェードアウトの終了赤
	_iFadeEdG = G;			// フェードアウトの終了緑
	_iFadeEdB = B;			// フェードアウトの終了青
	_iFadeEdA = 255;		// フェードアウトの終了アルファ

	_iFadeFrames = frame;	// フェードのフレーム数
	_iFadeCnt = 0;			// フェードのカウント
}

// フェードインアウトは終了したか？
//戻り値: 1 = フェード中, 0 = フェード終了(もしくは開始前)
int Fade::IsFade()
{
	if (_iFadeCnt < _iFadeFrames)
	{
		return 1; // フェード中
	}
	return 0; // フェード終了
}

// フレーム処理
bool Fade::Process()
{
	if(IsFade() != 0)
	{
		_iFadeCnt++; // フェードのカウンタを進める
		_iColorR = static_cast<int>(mymath::EasingLinear(static_cast<float>(_iFadeCnt), static_cast<float>(_iFadeStR), static_cast<float>(_iFadeEdR), static_cast<float>(_iFadeFrames))); // 赤の補間
		_iColorG = static_cast<int>(mymath::EasingLinear(static_cast<float>(_iFadeCnt), static_cast<float>(_iFadeStG), static_cast<float>(_iFadeEdG), static_cast<float>(_iFadeFrames))); // 緑の補間
		_iColorB = static_cast<int>(mymath::EasingLinear(static_cast<float>(_iFadeCnt), static_cast<float>(_iFadeStB), static_cast<float>(_iFadeEdB), static_cast<float>(_iFadeFrames))); // 青の補間
		_iAlpha  = static_cast<int>(mymath::EasingLinear(static_cast<float>(_iFadeCnt), static_cast<float>(_iFadeStA), static_cast<float>(_iFadeEdA), static_cast<float>(_iFadeFrames))); // アルファの補間
	}
	return true;
}

// 描画処理
bool Fade::Render()
{
	ApplicationBase* app = ApplicationBase::GetInstance();
	if(_iAlpha)
	{
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, _iAlpha);		// 半透明モードに設定
		DrawBox(0, 0, app->DispSizeW(), app->DispSizeH(), GetColor(_iColorR, _iColorG, _iColorB), TRUE); // カラーマスクの描画
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, _iAlpha);	// ブレンドモードを元に戻す
	}
	return true;
}