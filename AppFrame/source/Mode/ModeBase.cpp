/*********************************************************************/
// * \file   modebase.cpp
// * \brief  モードベースクラス
// *
// * \author 鈴木裕稀
// * \date   2025/12/15
// * \作業内容: 新規作成 鈴木裕稀　2025/12/15
/*********************************************************************/

#include "modebase.h"

ModeBase::ModeBase()
{
	_szName = "";
	_uid = 1;
	_layer = 0;

	_cntMode = 0;
	_tmMode = 0;
	_tmStep = 0;
	_tmModeBase = 0;
	_tmPauseBase = 0;
	_tmPauseStep = 0;
	_tmOldFrame = 0;

	SetCallPerFrame(1);
	SetCallOfCount(1);

	_iHandle = -1;
	_state = State::DONE;
	_iTimer = 0;
}

ModeBase::~ModeBase()
{
	// デストラクタ
}

// ModeServerに接続時、Process()の前に一度だけ呼ばれる
bool	ModeBase::Initialize()
{
	return true;
}

// ModeServerから削除される際、一度だけ呼ばれる
bool	ModeBase::Terminate()
{
	return true;
}

// --------------------------------------------------------------------------
/// @brief 毎フレーム呼ばれる。処理部を記述
// --------------------------------------------------------------------------
bool	ModeBase::Process()
{
	return	true;
}

// --------------------------------------------------------------------------
/// @brief 毎フレーム呼ばれる。描画部を記述
// --------------------------------------------------------------------------
bool	ModeBase::Render()
{
	return	true;
}

// 時間経過をさせる
void ModeBase::StepTime(unsigned long tmNow)
{
	// 時間経過処理
	if (_cntMode == 0) 
	{
		_tmMode = 0;
		_tmStep = 0;
		_tmModeBase = tmNow;
		_tmPauseBase = 0;
		_tmPauseStep = 0;
	}
	else
	{
		_tmMode = tmNow - _tmModeBase + _tmPauseStep;
		_tmStep = tmNow - _tmOldFrame;
	}
	_tmOldFrame = tmNow;
}

// カウントを進める
void ModeBase::StepCount()
{
	_cntMode++;
}
