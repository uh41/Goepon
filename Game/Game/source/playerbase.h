/*********************************************************************/
// * \file   playerbase.h
// * \brief  プレイヤーベースクラス
// *
// * \author 鈴木裕稀
// * \date   2025/12/15
// * \作業内容: 新規作成 鈴木裕稀　2025/12/15
/*********************************************************************/

#pragma once
#include "charabase.h"
#include "camera.h"
#include "appframe.h"

class PlayerBase : public CharaBase
{
	typedef CharaBase base;
public:
	virtual bool Initialize();
	virtual bool Terminate();
	virtual bool Process();
	virtual bool Render();

	vec::Vec3 GetMoveV() const { return _v; }
	void SetCamera(Camera* cam)  override { _cam = cam; }


protected:

	//アナログスティック関係
	float _fAnalogDeadZone; // アナログスティックの無効範囲
	float fLx, fLz, fRx, fRy;
	DINPUT_JOYSTATE di;
	//左スティックの値
	float lStickX, lStickZ;
	Camera* _cam;

	// 移動方向を決める
	vec3::Vec3 _v;

	// メモ読み込み
	std::string _memoFileName;
};

