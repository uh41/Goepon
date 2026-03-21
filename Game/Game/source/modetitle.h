#pragma once
#include "appframe.h"
#include "TitleTanuki.h"
#include "TitleCamera.h"
#include "applicationglobal.h"

namespace soundserver
{
	class SoundServer;
}

namespace ui
{
	static constexpr auto START_CONFIG_X = 0;
	static constexpr auto START_CONFIG_Y = 980;
}

class ModeTitle : public ModeBase
{
	typedef ModeBase base;
public:
	ModeTitle();
	virtual ~ModeTitle();
	virtual bool Initialize() override;
	virtual bool Terminate() override;
	virtual bool Process() override;
	virtual bool Render() override;

protected:
	at::spc<TitleTanuki> _player;
	TitleCamera* _cam;
		
	at::spc<soundserver::SoundServer> _soundServer;
	int _gbmHandle;
	int _startHandle;

private:
	int _bgHandle;
	int _titleHandle;

	// ロゴ落下アニメ用
	float _titleX		 = 30.0f;    // タイトルロゴのX位置
	float _titleY		 = -130.0f;  // タイトルロゴのY位置
	int   _titleW		 = 0;		 // タイトルロゴの幅
	int   _titleH		 = 0;		 // タイトルロゴの高さ
	float _titleVY       = 0.0f;     // タイトルロゴの落下速度
	float _titleTargetY  = -120.0f;  // タイトルロゴの最終的なY位置
	bool  _titleLanding  = false;    // タイトルロゴが落下中かどうか
	
};

