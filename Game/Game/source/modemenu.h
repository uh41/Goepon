/*********************************************************************/
// * \file   enemybase.h
// * \brief  エネミーベースクラス
// *
// * \author 鈴木裕稀
// * \date   2025/12/15
/*********************************************************************/

#pragma once
#include "appframe.h"
#include <string>
#include <vector>

class MenuItemBase;

class ModeMenu : public ModeBase
{
	typedef ModeBase base;
public:
	virtual bool Initialize();
	virtual bool Terminate();
	virtual bool Process();
	virtual bool Render();
	
	void Add(MenuItemBase* itemBase);
	void Clear();

protected:
	std::vector<MenuItemBase*> _items;

	int _cur_pos;
	int _cur_anim_cnt;
};

