#include "uibase.h"

// コンストラクタ
UiBase::UiBase()
{

}

// デストラクタ
UiBase::~UiBase()
{

}

// 初期化
bool UiBase::Initialize()
{
	return base::Initialize();
}

// 終了
bool UiBase::Terminate()
{
	return base::Terminate();
}

// 計算処理
bool UiBase::Process()
{
	return base::Process();
}

// 描画処理
bool UiBase::Render()
{
	return base::Render();
}