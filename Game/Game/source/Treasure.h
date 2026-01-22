#pragma once
#include "appframe.h"
#include "objectbase.h"

class Treasure : public ObjectBase
{
	typedef ObjectBase base;
public:

	// 宝箱の状態
	enum class OBJSTATUS
	{
		NONE, 
		NOMAR,
		OPEN,
	};
	OBJSTATUS _objStatus;

	// 基礎処理
	bool Initialize() override; // 初期化
	bool Terminate()  override; // 終了
	bool Process()    override; // 計算処理
	bool Render()     override; // 描画処理

	// 宝箱が開いているかどうか取得・設定
	bool IsOpen() const { return _isOpen; }
	void SetOpen(bool isOpen) { _isOpen = isOpen; }


protected:
	//int _handle;      // モデルハンドル
	int _attachIndex; // アタッチアニメーションインデックス
	bool _isOpen;	  // 宝箱が開いているかどうか
};

