#include "UiMakimono.h"
#include "playerbase.h"
#include "applicationglobal.h"
#include "appframe.h"

UiMakimono::UiMakimono()
{

}

UiMakimono::~UiMakimono()
{

}

bool UiMakimono::Initialize()
{
	base::Initialize();
	_handle = LoadGraph("res/Makimono/Makimono (1).png");
	return true;
}

bool UiMakimono::Terminate()
{
	base::Terminate();
	if(_handle >= 0)
	{
		DeleteGraph(_handle);
		_handle = -1;
	}
	return true;
}

bool UiMakimono::Process()
{
	base::Process();
	return true;
}

bool UiMakimono::Render()
{
	base::Render();

	// 画像未ロードなら何もしない
	if (_handle < 0)
	{
		return true;
	}

	// 画面サイズ
	const int screenW = ApplicationBase::GetInstance()->DispSizeW();
	const int screenH = ApplicationBase::GetInstance()->DispSizeH();

	// 画像サイズ
	int imgW = 0;
	int imgH = 0;
	GetGraphSize(_handle, &imgW, &imgH);

	// 余白は「UI仕様としての固定値」にする（メンバにしない）
	const int kPadding = 10;

	// 基準点（画面右下からの一定余白）※基準は動かさない
	const int anchorX = screenW - kPadding;
	const int anchorY = screenH - kPadding;

	// 画像位置（基準点を右下とみなして配置）
	const int kImageOffsetX = -1100;
	const int kImageOffsetY = +30;

	const int imageLeft = anchorX - imgW + kImageOffsetX;
	const int imageTop = anchorY - imgH + kImageOffsetY;


	const int bgW = anchorX - 1000;
	const int bgH = anchorY + 20;

	DrawBox(imageLeft - 20, imageTop + 30, bgW, bgH, GetColor(0, 0, 233), TRUE);
	DrawGraph(imageLeft, imageTop, _handle, TRUE);

	//============================
    // 巻物所持数（画像の右側に表示）
    //============================
	int makimonoCount = 0;
	if (_player != nullptr)
	{
		makimonoCount = _player->GetMakimonoCount();
	}
	const int kTextGapX = 12;               // 画像右端からの隙間
	const int textX = imageLeft + imgW + kTextGapX;
	const int textY = imageTop + (imgH / 2) - 10; // 縦はだいたい中央寄せ（微調整）

	SetFontSize(32);
	DrawFormatString(textX, textY, GetColor(255, 255, 255), "x %d", makimonoCount);


	return true;
}

