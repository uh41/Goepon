/*********************************************************************/
// * \file   camera.h
// * \brief  カメラクラス
// *
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

	// 追加: パン（位置・ターゲットを移動）・ターゲット方向にズーム
	void MoveBy(const VECTOR& delta);                 // カメラ位置とターゲットを同量だけ移動（パン）
	void ZoomTowardsTarget(float amount);             // ターゲット方向にカメラ位置を移動（プラスで近づく、マイナスで遠ざかる）
	void RotateAroundTarget(float deltaRad);         // ターゲットを中心にY軸回転（ラジアン）

	VECTOR _vPos;
	VECTOR _vTarget;
	float _fClipNear;
	float _fClipFar;
};