/*********************************************************************/
// * \file   player.h
// * \brief  人狸状態クラス
// *
// * \author 鈴木裕稀
// * \date   2025/12/15
// * \作業内容: 新規作成 鈴木裕稀　2025/12/15
//				vec::Vec3を使用するように修正　鈴木裕稀　2026/01/17
/*********************************************************************/

#pragma once
#include "playerbase.h"
#include "camera.h"
class Player : public PlayerBase
{
	typedef PlayerBase base;
public:
	virtual bool Initialize();
	virtual bool Terminate();
	virtual bool Process();
	virtual bool Render();


	void SetCamera(Camera* cam)  override { _cam = cam; if(_cam) { _camOffset = vec3::VSub(_cam->_vPos, _vPos); _camTargetOffset = vec3::VSub(_cam->_vTarget, _vPos); } }

protected:
	Camera* _cam;

	int _iAxisHoldCount;		// 十字キー水平入力保持カウント
	bool _bAxisUseLock;			// 十字キー水平入力ロックフラグ
	int _iAxisThreshold;		// 十字キー水平入力保持閾値

	vec::Vec3 _vAxisLockDir;

	// 円形移動用パラメータ
	float _arc_pivot_dist;		// 回転中心までの距離
	float _arc_turn_speed;		// 円形移動時の回転速度係数

	// カメラ追従用オフセット（カメラ位置 = _vPos + _camOffset）
	vec::Vec3 _camOffset;
	vec::Vec3 _camTargetOffset;

};

