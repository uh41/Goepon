/*********************************************************************/
// * \file   menuitembase.cpp
// * \brief  デバックメニュークラス
// *
// * \author 鈴木裕稀
// * \date   2025/12/15
// * \作業内容: 新規作成 鈴木裕稀　2025/12/15
/*********************************************************************/

#include "menuitembase.h"

// コンストラクタ
MenuItemBase::MenuItemBase(void* param, std::string text)
{
	_param = param;
	_text = text;
}

// デストラクタ
MenuItemBase::~MenuItemBase()
{
	// 特に何もしない
}

// 項目を決定したらこの関数が呼ばれる
// return int : 0 = メニュー継続, 1 = メニュー終了
int MenuItemBase::Selected()
{
	return 0;
}
