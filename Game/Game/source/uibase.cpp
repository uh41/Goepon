/*********************************************************************/
// * \file   uibase.cpp
// * \brief  UIベースクラス
// *
// * \author 鈴木裕稀
// * \date   2026/01/06
// * \作業内容: 新規作成 鈴木裕稀　2026/01/06
/*********************************************************************/

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
	_pos = vec::Vec3(0.0f, 0.0f, 0.0f);
	_visible = false;
	_size = 0.0f;
	_angle = 0.0f;

	return base::Initialize();
}

// 終了
bool UiBase::Terminate()
{
	return base::Terminate();
}

void UiBase::Show(const vec::Vec3& pos)
{
	_pos = pos;
	_visible = true;
}

void UiBase::Hide()
{
	_visible = false;
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