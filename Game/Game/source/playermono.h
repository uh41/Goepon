#pragma once
#include "playerbase.h"
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
};

