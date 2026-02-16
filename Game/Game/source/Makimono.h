#pragma once
#include "objectbase.h"
class Makimono : public ObjectBase
{
	typedef ObjectBase base;
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

