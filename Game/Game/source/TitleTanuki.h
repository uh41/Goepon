#pragma once
#include "CharaBase.h"	
class TitleTanuki: public CharaBase
{
	typedef CharaBase base;
public:
	bool Initialize() override;
	bool Terminate() override;
	bool Process() override;
	bool Render() override;

};

