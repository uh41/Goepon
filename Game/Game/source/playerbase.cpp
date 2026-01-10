/*********************************************************************/
// * \file   playerbase.cpp
// * \brief  プレイヤーベースクラス
// *
// * \author 鈴木裕稀
// * \date   2025/12/15
// * \作業内容: 新規作成 鈴木裕稀　2025/12/15
/*********************************************************************/

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
