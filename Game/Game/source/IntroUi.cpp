#include "IntroUi.h"
#include "modegame.h"	
#include "applicationmain.h"
#include "appframe.h"	

IntroUi::IntroUi()
{
	_ownerGame  = nullptr;
}

bool IntroUi::Initialize()
{
	base::Initialize();

	_handle = LoadGraph(ui::Gamestart);
	return true;
}

bool IntroUi::Terminate()
{
	base::Terminate();
	return true;
}

bool IntroUi::Process()
{
	base::Process();
	return true;
}

bool IntroUi::Render()
{
	base::Render();

	if(_ownerGame == nullptr)
	{
		return true;
	}

	// イントロ演出がアクティブでない場合は描画しない
	if(!_ownerGame->IsIntroActive())
	{
		return true;
	}

	const int sw = ApplicationBase::GetInstance()->DispSizeW();
	const int sh = ApplicationBase::GetInstance()->DispSizeH();

	const float elapsed = _ownerGame->GetIntroTimerSec();     // イントロ演出の経過時間を秒単位で取得
	const float duration = _ownerGame->GetIntroDurationSec(); // イントロ演出の総時間を秒単位で取得

	// フェード
	float alpha = 1.0f; // デフォルトは完全に不透明
	{
		const float fadeSec = 0.35f;  // フェードインにかける時間（秒）
		if(duration > 0.0f && fadeSec > 0.0f)
		{
			const float inA = elapsed / fadeSec;
			const float outA = (duration - elapsed) / fadeSec;

			float v = inA;
			if(outA < v)
			{
				v = outA;
			}

			alpha = mymath::Clamp(0.0f, 1.0f, v);
		}
	}

	const int a = StCas<int>(255.0f * alpha);

	// ロゴ
	if(_handle != -1)
	{
		const int x = ui::INTROLOGO_X;
		const int y = ui::INTROLOGO_Y;

		SetDrawBlendMode(DX_BLENDMODE_ALPHA, a);
		DrawGraph(x, y, _handle, TRUE);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	}
}