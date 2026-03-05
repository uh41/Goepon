#pragma once
#include "objectbase.h"
class TreasureBase : public ObjectBase
{
	typedef ObjectBase base;

public:
	enum class OBJSTATUS
	{
		NONE,
		NORMAL,
		OPEN,
	};

	virtual bool Initialize() override;
	virtual bool Terminate() override;
	virtual bool Process() override;
	virtual bool Render() override;

	auto GetHitCollisionFrame()  const { return _hitCollisionFrame;  }
	auto GetOpenCollisionFrame() const { return _openCollisionFrame; }
	
	bool IsOpen() const { return _isOpen; }
	// 実装はこっち
	//void SetOpen(bool isOpen) { _isOpen = isOpen; }
	void SetOpen(bool isOpen);

	// デバック用(モデルを表示/非表示)
	bool IsVisible() const { return _isVisible; }

	// モデルのワールド変換行列を作成
	MATRIX MakeModelMatrix() const;
protected:
	OBJSTATUS _objStatus;

	int _hitCollisionFrame;
	int _openCollisionFrame;

	bool _isOpen;
	bool _isVisible;

private:
	bool _openedNotified;

};

