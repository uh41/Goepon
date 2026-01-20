#include "fade.h"
#include "../mymath.h"
#include "../Application/applicationbase.h"

Fade::Fade()
{
	Initialize();
}

bool Fade::Initialize()
{

	_fadeColor = { 0, 0, 0, 0 }; // フェード用カラー配列(RGBA)

	_fadeStColor = { 0, 0, 0, 0 }; // フェードイン用開始カラー配列(RGBA)

	_fadeEdColor = { 0, 0, 0, 0 }; // フェードアウト用終了カラー配列(RGBA)

	_iFadeFrames = 0.0f;	// フェードのフレーム数
	_iFadeCnt = 0.0f;		// フェードのカウント
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
	_fadeColor = { R, G, B, alpha };
}

// 現在のカラーマスクからフェードイン
void Fade::FadeIn(float frame)
{

	_fadeStColor = _fadeColor;// フェードイン用開始カラー配列(RGBA)

	_fadeEdColor = { _fadeColor[0], _fadeColor[1], _fadeColor[3], 0 };// フェードアウト用終了カラー配列(RGBA)

	_iFadeFrames = frame;	// フェードのフレーム数
	_iFadeCnt = 0.0f;			// フェードのカウント
}

// 指定の色にフェードアウト
void Fade::FadeOut(int R, int G, int B, float frame)
{

	_fadeStColor = _fadeColor;// フェードイン用開始カラー配列(RGBA)

	_fadeEdColor = { R, G, B, 255 };// フェードアウト用終了カラー配列(RGBA)

	_iFadeFrames = frame;	// フェードのフレーム数
	_iFadeCnt = 0.0f;			// フェードのカウント
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
		for(int i = 0; i < 4; i++)
		{
			_fadeColor[i] = static_cast<int>(
				mymath::EasingLinear(
					static_cast<float>(_iFadeCnt),// カウント
					static_cast<float>(_fadeStColor[i]),// 開始カラー
					static_cast<float>(_fadeEdColor[i]),// 終了カラー
					static_cast<float>(_iFadeFrames))); // カラーの補間
		}
	}
	return true;
}

// 描画処理
bool Fade::Render()
{
	ApplicationBase* app = ApplicationBase::GetInstance();
	int alpha = _fadeColor[3];
	if(alpha)
	{
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);		// 半透明モードに設定
		DrawBox(0, 0, app->DispSizeW(), app->DispSizeH(), GetColor(_fadeColor[0], _fadeColor[1], _fadeColor[2]), TRUE); // カラーマスクの描画
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, alpha);	// ブレンドモードを元に戻す
	}
	return true;
}