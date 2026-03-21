#pragma once
#include "objectbase.h"
#include "appframe.h"
#include "camera.h"
#include "Light.h"

class MapBase : public ObjectBase
{
	typedef ObjectBase base;
public:
	virtual bool Initialize();
	virtual bool Terminate();
	virtual bool Process();
	virtual bool Render();

	void SetCamera(Camera* cam) override { _cam = cam; }

	const std::vector<mymath::BLOCKPOS>& GetBlockPosList() const { return _vBlockPos; }

	// マップIDのゲッター.セッター
	const std::string& GetMapId() const { return _mapName; }
	void SetMapId(const std::string& mapId) { _mapName = mapId; }
	
	//モデル読み込み
	virtual bool LoadModel(std::string fileName, std::string attachFrameName = "");

	// 外部の影キャスター描画
	void SetExternalShadowCasters(std::function<void()> caster) { _externalShadowCasters = caster; }
protected:
	Camera* _cam;
	// マップ用
	std::string _mapName;

	// シャドウマップ用

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
	std::array<VECTOR, 4> _ground_pos_list; // 地面のテクスチャ座標用のテーブル

	std::vector<mymath::BLOCKPOS> _vBlockPos;

	std::string _sPath;
	std::string _sJsonFile;
	std::string _sJsonObjectName;
	std::ifstream _iFile;

	// ライト情報
	Light _mainLight{ LightType::Directional };

	// 外部影キャスター
	std::function<void()> _externalShadowCasters;
};