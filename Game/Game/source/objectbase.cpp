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
#include "charaBase.h"


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
	// JSONから計算した位置を一度変数に保持
	vec::Vec3 newPos = vec::Vec3
	{
		j.at("translate").at("x").get<float>(),
		j.at("translate").at("z").get<float>(),
		-1.0f * j.at("translate").at("y").get<float>()
	};

	// UE rotate（度）を取得（既存の並びを踏襲）
	float rotXDeg = j.at("rotate").at("x").get<float>();
	float rotYDeg = j.at("rotate").at("z").get<float>(); // 水平回転(Yaw)として扱う
	float rotZDeg = j.at("rotate").at("y").get<float>();

	// スケール
	SetScale(vec::Vec3
		{
			j.at("scale").at("x").get<float>(),
			j.at("scale").at("z").get<float>(),
			j.at("scale").at("y").get<float>()
		});

	// 見た目の回転（モデル行列用）も入れるなら Euler に入れる（度→ラジアン）
	// ※ ObjectBase::ModelMatrixSetUp() は _vEulerAngle を使う
	SetEulerAngleDeg(vec::Vec3{ rotXDeg, rotYDeg, rotZDeg });
	ModelMatrixSetUp();

	// Dir（向きベクトル）は yaw から生成（-Z が forward 基準）
	float yaw = DEG2RAD(rotYDeg);
	vec::Vec3 dir = vec::Vec3
	{
		sinf(yaw),
		0.0f,
		-cosf(yaw)
	};

	// ObjectBase の位置・向き更新
	SetPos(newPos);
	SetDir(dir);

	// CharaBase を継承している派生クラスなら、そちらの位置・向きも更新
	if(auto ch = dynamic_cast<CharaBase*>(this))
	{
		ch->SetPos(newPos);
		ch->SetDir(dir);
	}
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
