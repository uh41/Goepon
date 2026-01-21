/*********************************************************************/
// * \file   playerbase.cpp
// * \brief  プレイヤーベースクラス
// *
// * \author 鈴木裕稀
// * \date   2025/12/15
// * \作業内容: 新規作成 鈴木裕稀　2025/12/15
/*********************************************************************/

#include "playerbase.h"


// 初期化
bool PlayerBase::Initialize()
{
	base::Initialize();
	// アナログスティックの設定関係
	_fAnalogDeadZone = 0.3f;
	return true;
}

// 終了
bool PlayerBase::Terminate()
{
	base::Terminate();
	return true;
}

// 計算処理
bool PlayerBase::Process()
{
	base::Process();

	// アナログスティックの状態取得
	{
		DINPUT_JOYSTATE di;
		GetJoypadDirectInputState(DX_INPUT_PAD1, &di);
		if (GetJoypadDirectInputState(DX_INPUT_PAD1, &di) == 0)
		{
			fLx = (float)di.X / 1000.f;
			fLz = (float)di.Y / 1000.f;
			fRx = (float)di.Z / 1000.f;
			fRy = (float)di.Rz / 1000.f;
		}
	}

	return true;
}

// 描画処理
bool PlayerBase::Render()
{
	base::Render();
	return true;
}
