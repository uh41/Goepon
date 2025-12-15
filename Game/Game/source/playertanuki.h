/*********************************************************************/
// * \file   playertanuki.h
// * \brief  ’Kó‘ÔƒNƒ‰ƒX
// *
// * \author —é–Ø—T‹H
// * \date   2025/12/15
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

	void SetCamera(Camera* cam)  override {_cam = cam;}

protected:
	Camera* _cam;
};

