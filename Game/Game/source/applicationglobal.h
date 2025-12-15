/*********************************************************************/
// * \file   applicationblocal.h
// * \brief  アプリケーショングローバルクラス
// *
// * \author 鈴木裕稀
// * \date   2025/12/15
// * \作業内容: 新規作成 鈴木裕稀　2025/12/15
/*********************************************************************/

#include "appframe.h"

class ApplicationGlobal
{
public:
	ApplicationGlobal();
	virtual ~ApplicationGlobal();

	bool Init();

	int _iCgCursor;
};

// 他のソースでgGlobalを使えるように
extern ApplicationGlobal gGlobal;
