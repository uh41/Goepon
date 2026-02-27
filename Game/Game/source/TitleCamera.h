#pragma once
#include "camera.h"
#include "TitleTanuki.h"
class TitleCamera : public Camera
{
	typedef Camera base;	
public: 
	virtual bool Initialize() override;
	virtual bool Process() override;
	virtual bool Render() override;

protected:
};

