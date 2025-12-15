/*********************************************************************/
// * \file   enemybase.cpp
// * \brief  エネミーベースクラス
// *
// * \author 鈴木裕稀
// * \date   2025/12/15
/*********************************************************************/

#include "enemybase.h"

// 初期化
bool EnemyBase::Initialize()
{
	base::Initialize();
	return true;
}

// 終了
bool EnemyBase::Terminate()
{
	base::Terminate();
	return true;
}

// 計算処理
bool EnemyBase::Process()
{
	base::Process();
	return true;
}

// 描画処理
bool EnemyBase::Render()
{
	base::Render();
	return true;
}


