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

	auto GetPos() const { return _vPos; }
	// 宝箱の当たり判定用フレーム取得
	auto GetHitCollisionFrame() const { return _hitCollisionFrame; }
	// 宝箱開閉用フレーム取得
	auto GetOpenCollisionFrame() const { return _openCollisionFrame; }	
	// モデルハンドルを取得
	auto GetModelHandle() const { return _handle; }

	// 宝箱が開いているかどうか取得・設定
	bool IsOpen() const { return _isOpen; }
	void SetOpen(bool isOpen) { _isOpen = isOpen; }

	// 現在の _vPos/_vDir/_vScale からモデル行列を生成
	MATRIX MakeModelMatrix() const;

protected:
	int _handle;                 // モデルハンドル
	int _hitCollisionFrame;      // 宝箱の当たり判定用フレーム
	int _openCollisionFrame;     // 宝箱開閉用フレーム
	int _attachIndex;            // アタッチアニメーションインデックス
	bool _isOpen;	             // 宝箱が開いているかどうか
};

