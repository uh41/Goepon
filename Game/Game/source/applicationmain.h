/*********************************************************************/
// * \file   applicationmain.h
// * \brief  アプリケーションメインクラス
// *
// * \author 鈴木裕稀
// * \date   2025/12/15
// * \作業内容: 新規作成 鈴木裕稀　2025/12/15
/*********************************************************************/

#pragma once
#include "appframe.h"

class ObjectServer;

class ApplicationMain : public ApplicationBase
{
	typedef ApplicationBase base;
public:
	static ApplicationMain* GetInstance();
	virtual bool Initialize(HINSTANCE hInstance);
	virtual bool Terminate();
	virtual bool Input();
	virtual bool Process();
	virtual bool Render();

	virtual bool AppWindowed() { return true; }
	virtual int DispSizeW() { return SCREEN_WIDTH; }
	virtual int DispSizeH() { return SCREEN_HEIGHT; }

	virtual bool BeforeDXLib_Init()override;

	ObjectServer* GetObjectServer() const { return _objectServer; }
protected:
	ObjectServer* _objectServer;
};
