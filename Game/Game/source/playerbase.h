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

	const VECTOR& GetInputVector() const { return _input_v; }
	float GetMoveSpeed() const { return _mv_speed; }
protected:

	Camera* _cam;
	bool _land;
	float _mv_speed;
	VECTOR _input_v;
};

