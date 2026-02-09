/*********************************************************************/
// * \file   ModeGoalConfirm.h
// * \brief  ゲームクリア確認クラス
// *
// * \author 石森虹大
// * \date   2026/2/9
// * \作業内容	: 新規作成 石森虹大　2026/2/9
//				
/*********************************************************************/

#pragma once
#include "appframe.h"
class ModeGoalConfirm :public ModeBase
{
	typedef ModeBase base;
public:
	enum class Result
	{
		None, 
		Yes,
		No,
	};

	explicit ModeGoalConfirm(Result* outResult); 

	bool Initialize() override;
	bool Terminate() override;
	bool Process() override;
	bool Render() override;

private:
	Result* _outResult;
	int _cursor;  // カーソル位置(0:Yes, 1:No)
	int _animCnt; // アニメーションカウンタ
};

