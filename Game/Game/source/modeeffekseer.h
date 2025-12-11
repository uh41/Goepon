#pragma once
#include "appframe.h"

#include <string>
#include <vector>

class ModeEffekseer : public ModeBase
{
	typedef ModeBase base;
public:
	virtual bool Initialize();
	virtual bool Terminate();
	virtual bool Process();
	virtual bool Render();

protected:
	int _effect_resource_handle;	// エフェクトファイルをロードするハンドル
	int _playing_effect_handle;		// ロードしたエフェクトファイルから、エフェクトを生成したもの

	// エフェクト位置
	float _position_x;
	float _position_y;
};

