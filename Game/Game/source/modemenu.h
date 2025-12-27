/*********************************************************************/
// * \file   modemenu.h
// * \brief  モードメニュークラス
// *
// * \author 鈴木裕稀
// * \date   2025/12/15
// * \作業内容: 新規作成 鈴木裕稀　2025/12/15
/*********************************************************************/

#pragma once
#include "appframe.h"

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

	// オーナーを設定（ModeGame ポインタを渡してメニューからカメラ操作させる）
	void SetOwner(void* owner) { _owner = owner; }

protected:
	std::vector<MenuItemBase*> _items;

	int _iCurPos;
	int _iCurAnimCnt;

	void* _owner;
};

