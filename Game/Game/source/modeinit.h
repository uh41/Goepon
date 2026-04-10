/*********************************************************************/
// * \file   modeinit.h
// * \brief  ŠwZƒƒSƒNƒ‰ƒX
// *
// * \author —é–Ø—T‹H
/*********************************************************************/
#pragma once
#include "appframe.h"

class ModeInit : public ModeBase
{
public:

	ModeInit();
	virtual ~ModeInit();
	virtual bool Initialize()override;
	virtual bool Terminate()override;
	virtual bool Process()override;
	virtual bool Render()override;
};

