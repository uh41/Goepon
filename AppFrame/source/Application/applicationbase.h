
#include "dxlib.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "../Mode//ModeServer.h"
#include "EffekseerForDXLib.h"

class ApplicationBase
{
public:
	ApplicationBase();
	virtual ~ApplicationBase();

	virtual bool Initialize(HINSTANCE hInstance);
	virtual bool Terminate();
	virtual bool Input();
	virtual bool Process();
	virtual bool Render();

	virtual bool BeforeDXLib_Init() { return true; }

	virtual bool AppWindowed() { return true; }
	virtual int DispSizeW() { return 640; }
	virtual int DispSizeH() { return 480; }

	static	ApplicationBase	*GetInstance() { return _lp_instance; }
	virtual int GetKey() { return _gKey; }
	virtual int GetTrg() { return _gTrg; }

protected:
	static	ApplicationBase	*_lp_instance;

	int		_gKey;
	int		_gTrg;

	ModeServer* _serverMode;

};
