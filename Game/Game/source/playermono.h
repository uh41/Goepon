#pragma once
#include "playerbase.h"
#include "camera.h"

class PlayerMono : public PlayerBase
{
	typedef PlayerBase base;
public:
	PlayerMono();
	virtual ~PlayerMono();

	virtual bool Initialize() override;
	virtual bool Terminate() override;
	virtual bool Process() override;
	virtual bool Render() override;

	void SetCamera(Camera* cam)  override { _cam = cam; if(_cam) { _camOffset = vec3::VSub(_cam->GetPos(), _vPos); _camTargetOffset = vec3::VSub(_cam->GetTarget(), _vPos); } }

	bool PlayerMonoSoundMove();

protected:

	Camera* _cam;

	// カメラ追従用オフセット
	vec::Vec3 _camOffset;
	vec::Vec3 _camTargetOffset;

};

