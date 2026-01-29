#pragma once
#include "appframe.h"
#include "objectbase.h"

class Treasure : public ObjectBase
{
	typedef ObjectBase base;
public:


	enum class OBJSTATUS
	{
		NONE,
		NOMAR,
		OPEN,
	};
	OBJSTATUS _objStatus;// オブジェクトの状態管理用


	bool Initialize() override; // 初期化
	bool Terminate()  override; // 終了
	bool Process()    override; // 処理
	bool Render()     override; // 描画

	auto GetPos() const { return _vPos; }

	auto GetHitCollisionFrame() const { return _hitCollisionFrame; }

	auto GetOpenCollisionFrame() const { return _openCollisionFrame; }

	auto GetModelHandle() const { return _handle; }


	bool IsOpen() const { return _isOpen; }
	// 実装はこっち
	//void SetOpen(bool isOpen) { _isOpen = isOpen; }
	// デバックとしてこちらを今使っている
	void SetOpen(bool isOpen)
	{
		_isOpen = isOpen;

		// ここを追加：開いたら見た目を消す
		if (_isOpen)
		{
			_isVisible = false;
		}
	}

	// デバック用(モデルを表示/非表示)
	bool IsVisible() const { return _isVisible; }


	MATRIX MakeModelMatrix() const;

protected:
	int _handle;                
	int _hitCollisionFrame;     
	int _openCollisionFrame;    
	int _attachIndex;           
	bool _isOpen;	             
	bool _isVisible;             // 表示フラグ
};

