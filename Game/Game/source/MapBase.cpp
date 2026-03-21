#include "MapBase.h"

// 初期化
bool MapBase::Initialize()
{
	if(!base::Initialize()) { return false; }
	_mapName.clear();

	// シャドウマップ追従の初期化
	return true;
}

// 終了
bool MapBase::Terminate()
{
	base::Terminate();
	// スカイスフィアモデルの削除
	if(_iHandleSkySphere >= 0)
	{
		ResourceServer::MV1DeleteModel(_iHandleSkySphere);
		_iHandleSkySphere = -1;
	}

	// メインマップモデルの削除
	if(_iHandleMap >= 0)
	{
		ResourceServer::MV1DeleteModel(_iHandleMap);
		_iHandleMap = -1;
	}

	// シャドウマップの削除（そのまま）
	if(_iHandleShadowMap >= 0)
	{
		DeleteShadowMap(_iHandleShadowMap);
		_iHandleShadowMap = -1;
	}

	// 個別モデルハンドルの削除
	for(auto& pair : _mModelHandle)
	{
		if(pair.second >= 0)
		{
			ResourceServer::MV1DeleteModel(pair.second);
		}
	}
	_mModelHandle.clear();

	// ベクターのクリア（容量ごと解放）
	_ground_vertex.clear();
	std::vector<VERTEX3D>().swap(_ground_vertex);         // メモリを確実に解放

	_ground_index.clear();
	std::vector<unsigned short>().swap(_ground_index);    // メモリを確実に解放

	_vBlockPos.clear();
	at::vet<mymath::BLOCKPOS>().swap(_vBlockPos);        // at::vet の場合も再構築で解放促進

	// ファイルストリームのクローズ
	if(_iFile.is_open())
	{
		_iFile.close();
	}

	return true;
}

// 更新
bool MapBase::Process()
{
	base::Process();
	return true;
}

// 描画
bool MapBase::Render()
{
	base::Render();
	return true;
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
