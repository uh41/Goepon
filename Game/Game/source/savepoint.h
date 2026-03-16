#pragma once
#include "objectBase.h"
class SavePoint : public ObjectBase
{
	typedef ObjectBase base;
public:
	SavePoint();
	virtual ~SavePoint() = default;
	virtual bool Initialize()override;
	virtual bool Terminate()override;
	virtual bool Process()override;
	virtual bool Render()override;

	int GetSavePointCollisionFrame() const { return _savePointCollisionFrame; }

	static int FindCollisonFrameAnyCase(int handle);
	MATRIX MakeModelMatrix() const;

protected:
	int _savePointCollisionFrame; // 当たり判定用フレーム
	int _attachIndex;             // モデルの当たり判定用フレームのアタッチ番号
	bool _isVisible;              // 表示フラグ
};

