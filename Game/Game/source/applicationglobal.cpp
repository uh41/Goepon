#include "applicationglobal.h"
#include "appframe.h"

// 実体
ApplicationGlobal gGlobal;

// コンストラクタ
ApplicationGlobal::ApplicationGlobal()
{
	_cg_cursor = -1;
}

// デストラクタ
ApplicationGlobal::~ApplicationGlobal()
{
	// 何もしない
}

// 初期化
bool ApplicationGlobal::Init()
{
	_cg_cursor = LoadGraph("res/cursor.png");

	return true;
}
