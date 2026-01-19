/*********************************************************************/
// * \file   applicationbase.h
// * \brief  アプリケーションベースクラス
// *
// * \author 鈴木裕稀
// * \date   2025/12/15
// * \作業内容: 新規作成 鈴木裕稀　2025/12/15
/*********************************************************************/

#include "../container.h"
#include "../Mode/ModeServer.h"
#include "../fade/fade.h"


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
	virtual int DispSizeW() { return 1920; }
	virtual int DispSizeH() { return 1080; }

	// ターゲットFPSの設定。デフォルトは60FPS
	virtual int TargetFps() const { return 60; }
	virtual int UseSync()   const { return false; }

	static	ApplicationBase	*GetInstance() { return _lp_instance; }
	virtual int GetKey() { return _gKey; }
	virtual int GetTrg() { return _gTrg; }

protected:
	static	ApplicationBase	*_lp_instance;
	static  Fade* _fade;

	int		_gKey;
	int		_gTrg;

	ModeServer* _serverMode;

};
