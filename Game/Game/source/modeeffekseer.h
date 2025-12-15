/*********************************************************************/
// * \file   modeeffeckseer.h
// * \brief  エフェクシアクラス
// *
// * \author 鈴木裕稀
// * \date   2025/12/15
// * \作業内容: 新規作成 鈴木裕稀　2025/12/15
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
	int _iEffectResourceHandle;	// エフェクトファイルをロードするハンドル
	int _iPlayingEffectHandle;		// ロードしたエフェクトファイルから、エフェクトを生成したもの

	VECTOR _vPos;
};

