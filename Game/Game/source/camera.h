#pragma once
#include "appframe.h"
#include "applicationmain.h"


class Camera
{
public:
	virtual bool Initialize();
	virtual bool Process();
	virtual bool Render();

	VECTOR _v_pos;
	VECTOR _v_target;
	float _clip_near;
	float _clip_far;
};

