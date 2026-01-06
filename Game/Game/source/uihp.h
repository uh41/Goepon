/*********************************************************************/
// * \file   uihp.h
// * \brief  UIHPクラス
// *
// * \author 鈴木裕稀
// * \date   2026/01/06
// * \作業内容: 新規作成 鈴木裕稀　2026/01/06
/*********************************************************************/


#pragma once
#include "uibase.h"

class PlayerBase;

class UiHp : public UiBase
{
	typedef UiBase base;

public:

	UiHp();
	virtual ~UiHp();
	virtual bool Initialize();
	virtual bool Terminate();
	virtual bool Process();
	virtual bool Render();

	void SetPlayer(PlayerBase* player) { _player = player; }

protected:
	PlayerBase* _player;

	float _fMaxHp;

	int _iBlock;
	int _iBlockW;
	int _iBlockH;
	int _iGap;	// ブロック間の隙間
	int _iPadding; // UI全体の余白
};

