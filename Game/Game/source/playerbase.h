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

	const vec::Vec3& GetInputVector() const { return _vInput; }
	float GetMoveSpeed() const { return _fMvSpeed; }
protected:

	Camera* _cam;
	bool _bLand;
	float _fMvSpeed;
	vec::Vec3 _vInput;

	// メモ読み込み
	std::string _memoFileName;
};

