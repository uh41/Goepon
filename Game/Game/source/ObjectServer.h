/*********************************************************************/
// * \file   ObjectServer.h
// * \brief  オブジェクトサーバークラス
// *
// * \author 石森虹大
// * \date   2026/1/5
// * \作業内容: 新規作成 石森虹大　2026/1/5
/*********************************************************************/


#pragma once
#include "objectbase.h"
#include"appframe.h"
class ObjectServer 
{
public:
	// コンストラクタ・デストラクタ
	ObjectServer(class ModeGame* obj);
	~ObjectServer();

	void AddObject(class ObjectBase* obj);
	void DeleteObject(class ObjectBase* obj);

	bool Intialize();		// 初期化
	bool Terminate();		// 終了処理
	bool ProcessInit();		// 処理の手前に呼び出す初期化
	bool Process();			// 処理
	bool Render();		    // 描画

	// オーナー
	class ModeGame* GetGame()const { return _game; }

	// プレイヤー
	class Player* GetPlayer() { return _player; }
	void SetPlayer(class Player* set) { _player = set; }

	// オブジェクトコンテナ
	std::vector<class ObjectBase*> GetObjects()const { return _objects; }

	// 関数を利用して、各クラスで追加されたアドレスを登録する
	std::vector<class CharaBase*>& GetCharas() { return _charas; }
	
	// オブジェクトのレイアウトを読み込み、オブジェクトを生成
	bool LoadDate(std::string stageName);

	//_objects _add _delete のコンテナが保持するアドレスをdelete、各コンテナのサイズを０にする
	bool ClearObject();

private:
	// オブジェクト　
	std::vector<class ObjectBase*> _objects;
	// 追加するオブジェクトのアドレス
	std::vector<class ObjectBase*> _addObj;
	// 削除するオブジェクトのアドレス
	std::vector<class ObjectBase*> _deleteObj;

	// 特定のクラスだけほしいので、アドレス登録用配列
	// 用途は当たり判定やObjectBase*型のポインタから特定のクラスしかできない処理をダウンキャストせずにするため
	std::vector<class CharaBase*> _charas;
	std::vector<class ObjectBase*>_enemys;
	
	// プレイヤークラス
	class Player* _player;

	// ModeGameが持つ機能や情報を利用するために、自身をnewしたModeGameクラスのアドレスと保持
	class ModeGame* _game;
};


