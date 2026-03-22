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

	// 外部影キャスターを先に解除（キャプチャしている参照を切る）
	_externalShadowCasters = nullptr;

	// スカイスフィアモデルの削除
	if (_iHandleSkySphere >= 0)
	{
		ResourceServer::MV1DeleteModel(_iHandleSkySphere);
		_iHandleSkySphere = -1;
	}

	// メインマップモデルの削除
	if (_iHandleMap >= 0)
	{
		ResourceServer::MV1DeleteModel(_iHandleMap);
		_iHandleMap = -1;
	}

	// シャドウマップの削除
	if (_iHandleShadowMap >= 0)
	{
		DeleteShadowMap(_iHandleShadowMap);
		_iHandleShadowMap = -1;
	}

	// 地面テクスチャハンドルの削除（※現状ここが抜けている）
	if (_ground_handle >= 0)
	{
		DeleteGraph(_ground_handle);
		_ground_handle = -1;
	}

	// 個別モデルハンドルの削除
	for (auto& pair : _mModelHandle)
	{
		if (pair.second >= 0)
		{
			ResourceServer::MV1DeleteModel(pair.second);
		}
	}
	_mModelHandle.clear();

	// ベクターの解放
	_ground_vertex.clear();
	std::vector<VERTEX3D>().swap(_ground_vertex);

	_ground_index.clear();
	std::vector<unsigned short>().swap(_ground_index);

	_vBlockPos.clear();
	at::vet<mymath::BLOCKPOS>().swap(_vBlockPos);

	// ファイルストリームのクローズ
	if (_iFile.is_open())
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
