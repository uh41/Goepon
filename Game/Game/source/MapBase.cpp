#include "MapBase.h"

// 初期化
bool MapBase::Initialize()
{
	if(!base::Initialize()) { return false; }
	_mapName.clear();
	return true;
}

// 終了
bool MapBase::Terminate()
{
	return base::Terminate();
}

// 更新
bool MapBase::Process()
{
	return base::Process();
}

// 描画
bool MapBase::Render()
{
	return base::Render();
}

// モデルの読み込み
bool MapBase::LoadModel(std::string fileName, std::string attachFrameName)
{
	// モデルの読み込み
	_iHandleMap = ResourceServer::MV1LoadModel(fileName.c_str());
	if(_handle < 0)
	{
		return false;
	}

	// アタッチフレーム名が指定されていれば、アタッチフレームのインデックスを取得
	_iFrameMapCollision = -1;
	if (!attachFrameName.empty())
	{
		_iFrameMapCollision = MV1SearchFrame(_iHandleMap, attachFrameName.c_str());
		if (_iFrameMapCollision >= 0)
		{
			MV1SetFrameVisible(_iHandleMap, _iFrameMapCollision, FALSE);
		}
	}
	return true;
}
