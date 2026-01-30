/*********************************************************************/
// * \file   applicationblocal.h
// * \brief  アプリケーショングローバルクラス
// *
// * \author 鈴木裕稀
// * \date   2025/12/15
// * \作業内容: 新規作成 鈴木裕稀　2025/12/15
/*********************************************************************/

#include "appframe.h"
#include "Player.h"
#include "Playertanuki.h"
#include "Enemy.h"
#include "map.h"


class ApplicationGlobal
{
public:
	ApplicationGlobal();
	virtual ~ApplicationGlobal();

	bool Init();    // 初期化

	int _iCgCursor;       // カーソル画像ハンドル

};

// 他のソースでgGlobalを使えるように
extern ApplicationGlobal gGlobal;
