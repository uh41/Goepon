/*********************************************************************/
// * \file   enemybase.h
// * \brief  エネミーベースクラス
// *
// * \author 鈴木裕稀
// * \date   2025/12/15
// * \作業内容: 新規作成 鈴木裕稀　2025/12/15
/*********************************************************************/

#pragma once
#include "charabase.h"
class EnemyBase : public CharaBase
{
	typedef CharaBase base;

public:

	virtual bool Initialize();
	virtual bool Terminate();
	virtual bool Process();
	virtual bool Render();

protected:

};

