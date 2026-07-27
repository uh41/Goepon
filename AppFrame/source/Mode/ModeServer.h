/*********************************************************************/
// * \file   modeserver.h
// * \brief  モードサーバークラス
// *
// * \author 鈴木裕稀
/*********************************************************************/

#pragma once

#include "../container.h"
#include "ModeBase.h"

class	ModeServer
{
	typedef	std::list<ModeBase*>		lstModeBase;

public:
	ModeServer();
	virtual	~ModeServer();
	static ModeServer		*_lpInstance;
	static ModeServer*GetInstance() { return (ModeServer*)_lpInstance; }

	int Add(ModeBase *mode, int layer, const char *name);		// 登録はするが、一度メインを回さないといけない
	int Del(ModeBase *mode);									// 削除予約
	ModeBase* Get(int uid);										// IDから検索
	ModeBase* Get(const char* name);							// 名前から検索
	int GetId(ModeBase* mode);									// ID取得
	int GetId(const char* name);								// ID取得
	const char* GetName(ModeBase* mode);						// 名前取得
	const char* GetName(int uid);								// 名前取得
	void Clear();												// 全削除
	int	LayerTop() { return INT32_MAX; }						// レイヤーの上限値

	// レイヤーのソート用
	static bool modeSort(const ModeBase *x, const ModeBase *y) 
	{
		return x->_layer < y->_layer;
	}

	int ProcessInit();											// プロセスを回すための初期化
	int Process();												// レイヤーの上の方から処理
	int ProcessFinish();										// プロセスを回した後の後始末

	int RenderInit();											// 描画を回すための初期化
	int Render();												// レイヤーの下の方から処理
	int RenderFinish();											// 描画を回した後の後始末

	int SkipProcessUnderLayer();		// 現Processで、今処理しているレイヤーより下のレイヤーは、処理を呼ばない
	int SkipRenderUnderLayer();			// 現Processで、今処理しているレイヤーより下のレイヤーは、描画を呼ばない
	int PauseProcessUnderLayer();		// 現Processで、今処理しているレイヤーより下のレイヤーは、時間経過を止める

private:
	int Release(ModeBase *mode);		// 削除＆delete
	bool IsDelRegist(ModeBase *mode);	// 削除予約されているか？
	bool IsAdd(ModeBase *mode);			// リストにあるか？

	lstModeBase		_vMode;			// モードリスト
	int				_uid_count;		// uidカウンタ
	lstModeBase		_vModeAdd;		// 追加予約
	lstModeBase		_vModeDel;		// 削除予約

	ModeBase		*_nowMode;		// 現在呼び出し中のモード
	ModeBase		*_skipProcessMode;	// このモードより下はProcessを呼ばない
	ModeBase		*_skipRenderMode;	// このモードより下はRenderを呼ばない
	ModeBase		*_pauseProcessMode;	// このモードより下はProcess時に時間経過をさせない

};
