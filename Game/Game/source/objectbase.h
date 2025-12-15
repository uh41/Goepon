/*********************************************************************/
// * \file   objectbase.h
// * \brief  オブジェクトベースクラス
// *
// * \author 鈴木裕稀
// * \date   2025/12/15
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

protected:
	Camera* _cam;
	int _handle;
	float _half_polygon_size;
	COLOR_U8 _diffuse;
	COLOR_U8 _specular;
	// uvのテーブル
	std::array<float, 4> _u_list;
	std::array<float, 4> _v_list;
};

