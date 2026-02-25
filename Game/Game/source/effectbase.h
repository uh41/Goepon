#pragma once
#include "charabase.h"
#include "appframe.h"

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

	virtual void PlayEffect(const vec::Vec3& pos) {}

	virtual bool StopPlaying();

	bool IsPlaying() const;

protected:
	bool _effekseerLaunche;
	int _playHandle;

};

