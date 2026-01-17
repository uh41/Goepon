/*********************************************************************/
// * \file   uihp.cpp
// * \brief  UIHPクラス
// *
// * \author 鈴木裕稀
// * \date   2026/01/06
// * \作業内容: 新規作成 鈴木裕稀　2026/01/06
/*********************************************************************/

#include "uihp.h"
#include "playerbase.h"
#include "applicationmain.h"

// コンストラクタ
UiHp::UiHp()
{
	_player = nullptr;
	_fMaxHp = 0.0f;
	_iBlock = 20;
	_iBlockW = 20;
	_iBlockH = 30;
	_iGap = 2;
	_iPadding = 15;// UI全体の余白
}

// デストラクタ
UiHp::~UiHp()
{

}

// 初期化
bool UiHp::Initialize()
{
	return base::Initialize();
}

// 終了
bool UiHp::Terminate()
{
	return base::Terminate();
}

// 計算処理
bool UiHp::Process()
{
	return base::Process();
}

// 描画処理
bool UiHp::Render()
{
	base::Render();

	if(_player == nullptr)
	{
		return true;
	}

	int screen_w = ApplicationBase::GetInstance()->DispSizeW();
	int screen_h = ApplicationBase::GetInstance()->DispSizeH();

	int total_width = _iBlock * _iBlockW + (_iBlock - 1) * _iGap;
	int start_x = screen_w - total_width - _iPadding;
	int start_y = screen_h - _iBlock - _iPadding;

	float hp = _player->GetHP();

	int filled = static_cast<int>((hp / _fMaxHp) * _iBlock + 0.5f);
	if(filled > _iBlock)
	{
		filled = _iBlock;
	}

	// 背景枠
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 192);
	DrawBox(start_x + 40, start_y, start_x + total_width, start_y + _iBlockH, GetColor(0, 0, 0), TRUE);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

	// HPブロック描画
	for(int i = 0; i < _iBlock; i++)
	{
		int x = static_cast<int>(start_x + 70.0f + i * (_iBlockW + _iGap));
		int y = start_y;
		if(i > filled)
		{
			DrawBox(x, y, x + _iBlockW, y + _iBlockH, GetColor(30, 200, 30), TRUE);
			DrawBox(x, y, x + _iBlockW, y + _iBlockH, GetColor(0, 0, 0), FALSE);
		}
		else
		{
			DrawBox(x, y, x + _iBlockW, y + _iBlockH, GetColor(80, 80, 80), TRUE);
			DrawBox(x, y, x + _iBlockW, y + _iBlockH, GetColor(0, 0, 0), FALSE);
		}
	}


	return true;
}