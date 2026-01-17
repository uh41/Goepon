/*********************************************************************/
// * \file   playertanuki.h
// * \brief  狸状態クラス
// *
// * \author 鈴木裕稀
// * \date   2025/12/15
// * \作業内容: 新規作成 鈴木裕稀　2025/12/15
//				vec::Vec3を使用するように修正　鈴木裕稀　2026 / 01 / 17
/*********************************************************************/

#pragma once
#include "playerbase.h"
#include "camera.h"

class PlayerTanuki : public PlayerBase
{
	typedef PlayerBase base;
public:

	bool Initialize() override;
	bool Terminate() override;
	bool Process() override;
	bool Render() override;

	void SetCamera(Camera* cam)  override { _cam = cam; if(_cam) { _camOffset = vec3::VSub(_cam->_vPos, _vPos); _camTargetOffset = vec3::VSub(_cam->_vTarget, _vPos); } }

protected:
	Camera* _cam;

	// カメラ追従用オフセット
	vec::Vec3 _camOffset;
	vec::Vec3 _camTargetOffset;
};

