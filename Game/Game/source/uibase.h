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

