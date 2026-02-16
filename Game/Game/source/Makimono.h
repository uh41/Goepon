#pragma once
#include "effectbase.h"
#include "appframe.h"	
class Makimono : public EffectBase
{
	typedef EffectBase base;
public:
	
	bool Initialize() override;
	bool Terminate() override;
	bool Process() override;
	bool Render() override;

	bool HaveMakimono() const { return haveMakimono; }
protected:
	bool  haveMakimono;
	bool  _isVisible;
	float _drawSize;

};

