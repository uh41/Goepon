#pragma once
#include "../container.h"
#include "../aliastemplate.h"

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

	void FadeIn(float frame);
	void FadeOut(int R, int G, int B, float frame);

	bool IsFade();		// フェード中かどうか

protected:


	at::art<int, 4> _fadeColor; // フェード用カラー配列(RGBA)

	at::art<int, 4> _fadeStColor; // フェードイン用開始カラー配列(RGBA)

	at::art<int, 4> _fadeEdColor; // フェードアウト用終了カラー配列(RGBA)

	// フェードのフレームカウント数
	float _iFadeFrames;	// フェードのフレーム数
	float _iFadeCnt;		// フェードのカウント

};

