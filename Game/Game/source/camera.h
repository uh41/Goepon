/*********************************************************************/
// * \file   camera.h
// * \brief  カメラクラス
// *
// * \author 鈴木裕稀
// * \date   2025/12/15
// * \作業内容: 新規作成 鈴木裕稀　2025/12/15
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

	VECTOR _vPos;
	VECTOR _vTarget;
	float _fClipNear;
	float _fClipFar;
};

