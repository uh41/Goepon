/*********************************************************************/
// * \file   objectbase.h
// * \brief  オブジェクトベースクラス
// *
// * \author 鈴木裕稀
// * \date   2025/12/15
// * \作業内容: 新規作成 鈴木裕稀　2025/12/15
//				vec::Vec3を使用するように修正　鈴木裕稀　2026/01/17
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
	void SetPos(const vec::Vec3& pos) { _vPos = pos; }
	void SetEulerAngle(const vec::Vec3& set) { _vEulerAngle = set; }
	void SetEulerAngleDeg(const vec::Vec3& set) { _vEulerAngle = { DEG2RAD(set.x), DEG2RAD(set.y), DEG2RAD(set.z) }; }
	void SetScale(const vec::Vec3& set) { _vScale = set; }

	//回転縮小平行移動を計算し、モデルに適用する
	void ModelMatrixSetUp();

protected:

	//モデルの当たり判定用フレームのアタッチ番号
	int _attachIndex;

	vec::Vec3 _vPos;				//ワールド座標

	vec::Vec3 _vEulerAngle;			//角度

	vec::Vec3 _vScale;				//拡縮

	std::string _sName;			    //名前

	//DamageData _damageData;		//ダメージデータ

	//STATE _state;				//状態

	Camera* _cam;
	int _iHandle;             // モデルハンドル
	float _half_polygon_size; // 地面ポリゴンの半分のサイズ
	COLOR_U8 _diffuse;        // 材質の色
	COLOR_U8 _specular;       // 材質の鏡面反射色
	// uvのテーブル
	std::array<float, 4> _u_list;
	std::array<float, 4> _v_list;
};

