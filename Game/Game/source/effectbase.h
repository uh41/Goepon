#pragma once
#include "charabase.h"

class EffectBase : public CharaBase
{
	typedef CharaBase base;
public:
	EffectBase();
	virtual ~EffectBase();

	virtual bool Initialize();
	virtual bool Terminate();
	virtual bool Process();
	virtual bool Render();

protected:
	bool _effekseerLaunche;

};

