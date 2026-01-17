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

	auto GetLand() const { return _bLand; }
	void SetLand(bool land) { _bLand = land; }

	void SetCamera(Camera* cam)  override { _cam = cam; }

	const VECTOR& GetInputVector() const { return _vInput; }
	float GetMoveSpeed() const { return _fMvSpeed; }
protected:

	//アナログスティック関係
	float _fAnalogDeadZone; // アナログスティックの無効範囲
	float fLx, fLz, fRx, fRy;
	DINPUT_JOYSTATE di;
	//左スティックの値
	float lStickX, lStickZ;
	Camera* _cam;
	bool    _bLand;
	float   _fMvSpeed;
	VECTOR  _vInput;
	// 移動方向を決める
	VECTOR _v = { 0,0,0 };

	// メモ読み込み
	std::string _memoFileName;
};

