#pragma once
#include "../container.h"

class Fade
{
public:
	Fade();
	~Fade() = default;

	bool Initialize();
	bool Terminate();
	bool Process();
	bool Render();

	// シングルトン
	static Fade* GetInstance();

	// 指定の色のカラーマスク
	void ColorMask(int R, int G, int B, int alpha);

	void FadeIn(int frame);
	void FadeOut(int R, int G, int B, int frame);

	int IsFade();		// フェード中かどうか

protected:
	int _iColorR;	// 赤
	int _iColorG;	// 緑
	int _iColorB;	// 青
	int _iAlpha;	// アルファ値

	// フェードイン用
	int _iFadeStR;	// フェードインの開始赤
	int _iFadeStG;	// フェードインの開始緑
	int _iFadeStB;	// フェードインの開始青
	int _iFadeStA;	// フェードインの開始アルファ

	// フェードアウト用
	int _iFadeEdR;	// フェードアウトの終了赤
	int _iFadeEdG;	// フェードアウトの終了緑
	int _iFadeEdB;	// フェードアウトの終了青
	int _iFadeEdA;	// フェードアウトの終了アルファ

	// フェードのフレームカウント数
	int _iFadeFrames;	// フェードのフレーム数
	int _iFadeCnt;		// フェードのカウント

};

