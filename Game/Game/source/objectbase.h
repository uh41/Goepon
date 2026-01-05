/*********************************************************************/
// * \file   objectbase.h
// * \brief  オブジェクトベースクラス
// *
// * \author 鈴木裕稀
// * \date   2025/12/15
// * \作業内容: 新規作成 鈴木裕稀　2025/12/15
/*********************************************************************/

#pragma once
#include "camera.h"
#include "appframe.h" 

namespace
{
	constexpr auto DEFAULT_SIZE = 100.0f;
	constexpr auto GROUND_Z = 50.0f;
	constexpr auto GROUND_X = 100.0f;
	constexpr auto GROUND_POLYGON_SIZE = 300.0f;
}

class ObjectBase
{
public:
	virtual bool Initialize();
	virtual bool Terminate();
	virtual bool Process();
	virtual bool Render();

	virtual void SetCamera(Camera* cam) { _cam = cam; }
	//jsonデータの格納
	virtual void SetJsonDataUE(nlohmann::json j);

	// セッター
	void SetPos(const VECTOR& pos) { _pos = pos; }
	void SetEulerAngle(const VECTOR& set) { _eulerAngle = set; }
	void SetEulerAngleDeg(const VECTOR& set) { _eulerAngle = { DEG2RAD(set.x), DEG2RAD(set.y), DEG2RAD(set.z) }; }
	void SetScale(const VECTOR& set) { _scale = set; }

	//回転縮小平行移動を計算し、モデルに適用する
	void ModelMatrixSetUp();

protected:

	//モデルのハンドル
	int _handle;
	//モデルの当たり判定用フレームのアタッチ番号
	int _attachIndex;

	VECTOR _pos;				//ワールド座標

	VECTOR _eulerAngle;			//角度

	VECTOR _scale;				//拡縮

	std::string _name;			//名前

	//DamageData _damageData;		//ダメージデータ

	//STATE _state;				//状態

	Camera* _cam;
	int _iHandle;
	float _half_polygon_size;
	COLOR_U8 _diffuse;
	COLOR_U8 _specular;
	// uvのテーブル
	std::array<float, 4> _u_list;
	std::array<float, 4> _v_list;
};

