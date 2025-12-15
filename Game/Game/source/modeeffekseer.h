/*********************************************************************/
// * \file   modeeffeckseer.h
// * \brief  エフェクシアクラス
// *
// * \author 鈴木裕稀
// * \date   2025/12/15
/*********************************************************************/

#pragma once
#include "appframe.h"

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

