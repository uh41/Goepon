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
	if(_handle < 0)
	{
		return true;
	}

	// 画面サイズ
	const int screen_w = ApplicationBase::GetInstance()->DispSizeW();
	const int screen_h = ApplicationBase::GetInstance()->DispSizeH();

	// 画像サイズ
	int img_w = 0;
	int img_h = 0;
	GetGraphSize(_handle, &img_w, &img_h);

	const int draw_w = static_cast<int>(img_w);
	const int draw_h = static_cast<int>(img_h);

	// 右下座標（画像の左上座標）
	const int x = screen_w - draw_w - _padding;
	const int y = screen_h - draw_h - _padding;

	// 表示位置微調整（現状コードに合わせる）
	const int ui_x = x - 1100;
	const int ui_y = y + 30;

	// 背景を描画
	DrawBox(x - 1200, y + 50, x - 900 + draw_w, y + draw_h + 20, GetColor(0, 0, 233), TRUE);
	
	// 拡大縮小して描画（_scale=1.0f なら等倍）
	DrawGraph(ui_x, ui_y, _handle, TRUE);

	return true;
}

