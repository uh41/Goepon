/*********************************************************************/
// * \file   playerbase.h
// * \brief  プレイヤーベースクラス
// *
// * \author 鈴木裕稀
// * \date   2025/12/15
/*********************************************************************/

#pragma once
#include "charabase.h"
#include "camera.h"

class PlayerBase : public CharaBase
{
	typedef CharaBase base;
public:
	virtual bool Initialize();
	virtual bool Terminate();
	virtual bool Process();
	virtual bool Render();

	auto GetLand() const { return _land; }
	void SetLand(bool land) { _land = land; }

	void SetCamera(Camera* cam)  override { _cam = cam; }

	const VECTOR& GetInputVector() const { return _inputV; }
	float GetMoveSpeed() const { return _mvSpeed; }
protected:

	Camera* _cam;
	bool _land;
	float _mvSpeed;
	VECTOR _inputV;
};

