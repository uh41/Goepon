/*********************************************************************/
// * \file   camera.h
// * \brief  カメラクラス
// *
// * \author 鈴木裕稀
// * \date   2025/12/15
// * \作業内容: 新規作成                       鈴木裕稀　2025/12/15
//				vec::Vec3を使用するように修正　鈴木裕稀　2026/01/17

/*********************************************************************/

#pragma once
#include "appframe.h"
#include "applicationmain.h"

class Camera
{
public:
	virtual bool Initialize();
	virtual bool Process();
	virtual bool Render();

	//パン（位置・ターゲットを移動）・ターゲット方向にズーム
	void MoveBy(const vec::Vec3& delta);                 // カメラ位置とターゲットを同量だけ移動（パン）
	void ZoomTowardsTarget(float amount);             // ターゲット方向にカメラ位置を移動（プラスで近づく、マイナスで遠ざかる）
	void RotateAroundTarget(float deltaRad);         // ターゲットを中心にY軸回転（ラジアン）
	void SetForvScele(float forv) { _fForvScale = forv; } //視野角設定
	float GetForvScale() const { return _fForvScale; }

	vec::Vec3 _vPos;
	vec::Vec3 _vTarget;
	float _fClipNear;
	float _fClipFar;
	float _fForvScale;
};