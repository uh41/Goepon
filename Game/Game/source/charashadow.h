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

	void SetTargetChara(CharaBase* chara) { _chara = chara; }
	void SetTexturPath(const std::string& path) { _texturePath = path; }

protected:
	CharaBase* _chara;
	std::string _texturePath;
	float _fYOffset;// シャドウのYオフセット
	float _fAlpha;// シャドウの透明度
	float _fSizeFactor;// シャドウのサイズ倍率
};

