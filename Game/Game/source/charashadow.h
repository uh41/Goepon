#pragma once
#include "charabase.h"
#include "appframe.h"

class CharaShadow : public CharaBase
{
	typedef CharaBase base;
public:

	CharaShadow();
	virtual ~CharaShadow();

	virtual bool Initialize() override;
	virtual bool Terminate() override;
	virtual bool Process() override;
	virtual bool Render() override;

	// ‰e‚ğ’Ç]‚³‚¹‚éƒLƒƒƒ‰‚ğİ’è
	void SetTargetChara(CharaBase* chara) { _Chara = chara; }

	// ‰e‚ÌƒXƒP[ƒ‹‚ğ’²®
	void SetScale(float scale) { _fScale = scale; }

protected:
	CharaBase* _Chara;
	float _fScale;
};

