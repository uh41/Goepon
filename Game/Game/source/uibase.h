/*********************************************************************/
// * \file   uibase.h
// * \brief  UIƒx[ƒXƒNƒ‰ƒX
// *
// * \author —é–Ø—T‹H
// * \date   2026/01/06
// * \ì‹Æ“à—e: V‹Kì¬ —é–Ø—T‹H@2026/01/06
/*********************************************************************/

#pragma once
#include "charabase.h"
class UiBase : public CharaBase
{
	typedef CharaBase base;
public:
	UiBase();
	virtual ~UiBase();

	virtual bool Initialize();
	virtual bool Terminate();
	virtual bool Process();
	virtual bool Render();

};

