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
	// UIの開始位置
	static constexpr auto MENU_UI_X = 50;
	static constexpr auto MENU_UI_Y = 940;
	static constexpr auto MENU_ITEM_SPACING = 550; // 間のスペース
}

class ModeTitle : public ModeBase
{
	typedef ModeBase base;
public:
	virtual ~ModeTitle();
	virtual bool Initialize() override;
	virtual bool Terminate() override;
	virtual bool Process() override;
	virtual bool Render() override;

protected:
	at::spc<TitleTanuki> _player;
	TitleCamera* _cam = nullptr;
		
	at::spc<soundserver::SoundServer> _soundServer;
	int _gbmHandle;
	int _startHandle;

private:
	// メニュー選択の状態
	enum class MenuItem
	{
		Start,
		Exit,
	};
	MenuItem _menu = MenuItem::Start; // デフォルトは「スタート」


	// ハンドル関係
	// タイトルロゴと背景のハンドル
	int _bgHandle;
	int _titleHandle;

	// メニュー選択ハンドル
	int _startYesHandle = -1;
	int _startNoHandle  = -1;
	int _exitYesHandle  = -1;
	int _exitNoHandle	= -1;

	// ふすまハンドル
	int _fusumaRighetHandle = -1;
	int _fusumaLeftHandle   = -1;

	// ロゴ落下アニメ用
	float _titleX		 = 30.0f;    // タイトルロゴのX位置
	float _titleY		 = -130.0f;  // タイトルロゴのY位置
	int   _titleW		 = 0;		 // タイトルロゴの幅
	int   _titleH		 = 0;		 // タイトルロゴの高さ
	float _titleVY       = 0.0f;     // タイトルロゴの落下速度
	float _titleTargetY  = -120.0f;  // タイトルロゴの最終的なY位置
	bool  _titleLanding  = false;    // タイトルロゴが落下中かどうか

	// ふすま演出
	enum class FusumaState
	{
		None,
		closing,
		closed,	
	};
	FusumaState _fusumaState = FusumaState::None;

	// ふすま演出関連
	void StartFusumaClose();	// ふすまを閉じる演出開始
	bool ProcessFusumaClose();  // ふすまを閉じる演出処理。演出中はtrueを返す
	void RenderFusuma() const;  // ふすまを描画する

	int   _fusumaW = 0;			        // ふすまの幅
	int   _fusumaH = 0;			        // ふすまの高さ
	float _fusumaLeftX  = 0.0f;         // 左のふすまのX位置
	float _fusumaRightX = 0.0f;         // 右のふすまのX位置
	float _fusumaY		 = 0.0f;        // ふすまのY位置	
	float _fusumaCnt = 0.0f;            // 経過フレーム
	float _fusumaFrames = 30.0f;        // 閉じるのにかけるフレーム
	int   _fusumaClosedWaitCnt = 0;     // ふすまが閉じた状態での待機フレーム
	int   _fusumaClosedWaitFrames = 30; // ふすまが閉じた状態で待機するフレーム

	bool _startedGlobalInit = false;
};

