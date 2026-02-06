#include "MapBase.h"

// 初期化
bool MapBase::Initialize()
{
	if(!base::Initialize()) { return false; }
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
	if(!attachFrameName.empty())
	{
		// アタッチフレームのインデックスを取得
		_iFrameMapCollision = MV1SearchFrame(_handle, attachFrameName.c_str());
		// アタッチフレームが見つからなかった場合はエラー
		if(_iFrameMapCollision >= 0)
		{
			// アタッチフレームを非表示にする
			MV1SetFrameVisible(_handle, _iFrameMapCollision, FALSE);
		}
	}
	return true;
}
