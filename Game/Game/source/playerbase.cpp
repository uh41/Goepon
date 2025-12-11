#include "playerbase.h"

// 初期化
bool PlayerBase::Initialize()
{
	base::Initialize();
	return true;
}

// 終了
bool PlayerBase::Terminate()
{
	base::Terminate();
	return true;
}

// 計算処理
bool PlayerBase::Process()
{
	base::Process();
	return true;
}

// 描画処理
bool PlayerBase::Render()
{
	base::Render();
	return true;
}
