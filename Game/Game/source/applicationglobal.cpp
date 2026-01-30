/*********************************************************************/
// * \file   applicationblocal.cpp
// * \brief  アプリケーショングローバルクラス
// *
// * \author 鈴木裕稀
// * \date   2025/12/15
// * \作業内容: 新規作成 鈴木裕稀　2025/12/15
/*********************************************************************/

#include "applicationglobal.h"
#include "appframe.h"


// 実体
ApplicationGlobal gGlobal;


// コンストラクタ
ApplicationGlobal::ApplicationGlobal()
{
	_iCgCursor = -1;
}

// デストラクタ
ApplicationGlobal::~ApplicationGlobal()
{
	// 何もしない
}

// 初期化
bool ApplicationGlobal::Init()
{
	SetUseASyncLoadFlag(TRUE);
	_iCgCursor = LoadGraph("res/cursor.png");
	// キャラクター関連モデル読み込み
	ResourceServer::MV1LoadModel("res/Tanuhuman/TanuHuman.mv1");
	ResourceServer::MV1LoadModel("res/Tanuki/SK_goepon_multimotion.mv1");
	ResourceServer::MV1LoadModel("res/PoorEnemyMelee/bushi_0114taiki.mv1");
	// マップ関連モデル読み込み
	ResourceServer::MV1LoadModel("res/SkySphere/skysphere.mv1");
	ResourceServer::MV1LoadModel("res/Ground/Ground.mv1");
	ResourceServer::MV1LoadModel("res/map/SM_map.mv1");
	SetUseASyncLoadFlag(FALSE);

	return true;
}
