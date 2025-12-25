/*********************************************************************/
// * \file   map.h
// * \brief  マップクラス
// *
// * \author 鈴木裕稀
// * \date   2025/12/15
// * \作業内容: 新規作成				鈴木裕稀　2025/12/15
//				Jsonファイル読み込み　	鈴木裕稀　2025/12/25
/*********************************************************************/

#pragma once
#include "objectbase.h"
#include "appframe.h"
#include "camera.h"

class Map : public ObjectBase
{
	typedef ObjectBase base;
public:
	virtual bool Initialize();
	virtual bool Terminate();
	virtual bool Process();
	virtual bool Render();

	auto& GetHandleMap() { return _iHandleMap; }
	auto GetHandleSkySphere() const { return _iHandleSkySphere; }
	auto GetFrameMapCollision() const { return _iFrameMapCollision; }
	auto GetHandleShadowMap() const { return _iHandleShadowMap; }

	void SetCamera(Camera* cam) override { _cam = cam; }

protected:
	Camera* _cam;
	// マップ用
	std::map<std::string, int> _mModelHandle;// std::map<マップ名, ハンドル>
	int _iHandleMap;// std::map<マップ名, ハンドル>
	int _iHandleSkySphere;
	int _iFrameMapCollision;

	// シャドウマップ用
	int _iHandleShadowMap;

	// 地面のテクスチャ
	int _ground_handle;
	std::vector<VERTEX3D> _ground_vertex;
	std::vector<unsigned short> _ground_index;

	// 地面全体の中心を原点にする
	float _start_x;
	float _start_z;
	// 頂点用の固定情報
	VECTOR _ground_normal;
	//地面の正方形用のテーブル
	VECTOR _ground_pos_0;
	VECTOR _ground_pos_1;
	VECTOR _ground_pos_2;
	VECTOR _ground_pos_3;
	std::array<VECTOR, 4> _ground_pos_list;

	std::vector<mymath::BLOCKPOS> _vBlockPos;

	std::string _sPath;
	std::string _sJsonFile;
	std::string _sJsonObjectName;
	std::ifstream _iFile;
};

