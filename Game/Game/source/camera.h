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
	virtual bool Terminate();
	virtual bool Process();
	virtual bool Render();

	//パン（位置・ターゲットを移動）・ターゲット方向にズーム
	void MoveBy(const vec::Vec3& delta);                 // カメラ位置とターゲットを同量だけ移動（パン）
	void ZoomTowardsTarget(float amount);             // ターゲット方向にカメラ位置を移動（プラスで近づく、マイナスで遠ざかる）
	void RotateAroundTarget(float deltaRad);         // ターゲットを中心にY軸回転（ラジアン）

	// 視野角の設定・取得
	void SetForvScele(float forv) { _forvScale = forv; } //視野角設定
	float GetForvScale() const { return _forvScale; }

	// カメラ位置の設定・取得
	void SetPos(const vec::Vec3& pos) { _vPos = pos; }
	const vec::Vec3& GetPos() const { return _vPos; }

	// ターゲット位置の設定・取得
	void SetTarget(const vec::Vec3& target) { _vTarget = target; }
	const vec::Vec3& GetTarget() const { return _vTarget; }

	// クリップ距離の設定・取得
	void SetClipNear(float nearClip) { _clipNear = nearClip; }
	float GetClipNear() const { return _clipNear; }

	void SetClipFar(float farClip) { _clipFar = farClip; }
	float GetClipFar() const { return _clipFar; }

protected:
	vec::Vec3 _vPos;
	vec::Vec3 _vTarget;
	float _clipNear;
	float _clipFar;
	float _forvScale;	// 視野角のスケール（値が大きいほど広く見える）
};