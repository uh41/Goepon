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

	// デバック用(モデルを表示/非表示)
	bool IsVisible() const { return _isVisible; }
	void SetVisible(bool isVisible) { _isVisible = isVisible; }

	int HaveMakimono() const { return haveMakimono; }
	void SetHaveMakimono(int have) { haveMakimono = have; }
protected:
	int  haveMakimono;
	bool  _isVisible;
	float _drawSize;

};

