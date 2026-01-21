#pragma once
#include "appframe.h"
class Treasure
{
public:

	// 宝箱の状態
	enum class STATUS
	{
		NONE, 
		NOMAR,
		OPEN,
	};
	STATUS _status;

	// 基礎処理
	bool Initialize(); // 初期化
	bool Terminate(); // 終了
	bool Process();   // 計算処理
	bool Render();    // 描画処理

	// 宝箱の位置取得・設定
	const vec::Vec3& GetPos() const { return _vPos; }
	void SetPos(const vec::Vec3& pos) { _vPos = pos; }

	// 宝箱の向き取得・設定
	const vec::Vec3& GetDir() const { return _vDir; }
	void SetDir(const vec::Vec3& dir) { _vDir = dir; }

	// 宝箱が開いているかどうか取得・設定
	bool IsOpen() const { return _isOpen; }
	void SetOpen(bool isOpen) { _isOpen = isOpen; }


protected:
	int _handle;    // モデルハンドル
	int _attachIndex; // アタッチアニメーションインデックス
	vec::Vec3 _vPos;  // 宝箱の位置
	vec::Vec3 _vDir;  // 宝箱の向き
	
	bool _isOpen;	  // 宝箱が開いているかどうか
};

