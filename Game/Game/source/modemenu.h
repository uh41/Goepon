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

