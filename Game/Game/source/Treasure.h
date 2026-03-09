/*********************************************************************/
// * \file   treasure.h
// * \brief  お宝クラス
// *
// * \author 石森虹大
// * \date   2026/1/25
// * \作業内容: 新規作成 石森虹大 2026/1/25
//			 
/*********************************************************************/
#pragma once
#include "appframe.h"
#include "objectbase.h"
#include "TreasureBase.h"

class Treasure : public TreasureBase
{
	typedef TreasureBase base;
public:
	bool Initialize() override; // 初期化
	bool Terminate()  override; // 終了
	bool Process()    override; // 処理
	bool Render()     override; // 描画

protected:    
	int _attachIndex;    
};

