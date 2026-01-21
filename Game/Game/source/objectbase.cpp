/*********************************************************************/
// * \file   objectbase.cpp
// * \brief  オブジェクトベースクラス
// *
// * \author 鈴木裕稀
// * \date   2025/12/15
// * \作業内容: 新規作成 鈴木裕稀　2025/12/15
/*********************************************************************/

#include "objectbase.h"
#include"DxLib.h"


// 初期化
bool ObjectBase::Initialize()
{
	_cam = nullptr;
	_handle = -1;
	_half_polygon_size = 0.0f;
	_diffuse = GetColorU8(255, 255, 255, 255);
	_specular = GetColorU8(0, 0, 0, 0);
	_u_list = { 0.0f, 0.0f, 1.0f, 1.0f };
	_v_list = { 0.0f, 1.0f, 0.0f, 1.0f };

	return true;
}

// 終了
bool ObjectBase::Terminate()
{
	return true;
}

// 計算処理
bool ObjectBase::Process()
{
	return true;
}
// 描画処理
bool ObjectBase::Render()
{
	return true;
}

//座標などの読み込み
void ObjectBase::SetJsonDataUE(nlohmann::json j)
{
	SetPos(vec::Vec3
	{
	j.at("translate").at("x").get<float>(),
	j.at("translate").at("z").get<float>(),
	-1.0f * j.at("translate").at("y").get<float>()
	});
	SetEulerAngleDeg(vec::Vec3
	{
		j.at("rotate").at("x").get<float>(),
		j.at("rotate").at("z").get<float>(),
		j.at("rotate").at("y").get<float>()
	});
	SetScale(vec::Vec3
	{
		j.at("scale").at("x").get<float>(),
		j.at("scale").at("z").get<float>(),
		j.at("scale").at("y").get<float>()
	});
	ModelMatrixSetUp();
}

// モデルに角度と移動値を判定する
void ObjectBase::ModelMatrixSetUp()
{
	MATRIX matrix = MGetIdent();
	matrix = MMult(matrix, MGetRotX(_vEulerAngle.x));
	matrix = MMult(matrix, MGetRotZ(_vEulerAngle.z));
	matrix = MMult(matrix, MGetRotY(_vEulerAngle.y + PI));
	MV1SetMatrix(_handle, matrix);

	MV1RefreshCollInfo(_handle, _attachIndex);
}
