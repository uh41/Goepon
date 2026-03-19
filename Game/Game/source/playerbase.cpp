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

	_makimonoCount = 0;

	// 移動速度設定
	_v = { 0,0,0 };

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
		// まずはゼロクリア（未接続時も値が残らないようにする）
		fLx = 0.0f;
		fLz = 0.0f;
		fRx = 0.0f;
		fRy = 0.0f;

		// ジョイパッドがあればジョイパッド優先
		if(GetJoypadNum() > 0)
		{
			DINPUT_JOYSTATE di;
			if(GetJoypadDirectInputState(DX_INPUT_PAD1, &di) == 0)
			{
				fLx = (float)di.X / 1000.0f;
				fLz = (float)di.Y / 1000.0f;
				fRx = (float)di.Z / 1000.0f;
				fRy = (float)di.Rz / 1000.0f;
			}
		}
		else
		{
			// ジョイパッド無しならキーボード入力で左スティックを生成（矢印キー）
			float x = 0.0f;
			float z = 0.0f;

			if(CheckHitKey(KEY_INPUT_LEFT))
			{
				x = -1.0f;
			}
			if(CheckHitKey(KEY_INPUT_RIGHT))
			{
				x = 1.0f;
			}
			if(CheckHitKey(KEY_INPUT_UP))
			{
				z = -1.0f;
			}
			if(CheckHitKey(KEY_INPUT_DOWN))
			{
				z = 1.0f;
			}

			fLx = x;
			fLz = z;
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

void PlayerBase::AddMakimono(int addCount)
{
	_makimonoCount += addCount;
	
	// 0未満にならないようにする
	if(_makimonoCount < 0)
	{
		_makimonoCount = 0;
	}
}

void PlayerBase::SubMakimono(int subCount)
{
	AddMakimono(-subCount);
}
