/*********************************************************************/
// * \file   applicationblocal.cpp
// * \brief  アプリケーショングローバルクラス
// *
// * \author 鈴木裕稀
// * \date   2025/12/15
// * \作業内容: 新規作成 鈴木裕稀　2025/12/15
/*********************************************************************/

#include "applicationglobal.h"
#include "appframe.h"

// 実体
ApplicationGlobal gGlobal;

// コンストラクタ
ApplicationGlobal::ApplicationGlobal()
{
	_iCgCursor = -1;
}

// デストラクタ
ApplicationGlobal::~ApplicationGlobal()
{
	// 何もしない
}

// 初期化
bool ApplicationGlobal::Init()
{
	_iCgCursor = LoadGraph("res/cursor.png");

	return true;
}
