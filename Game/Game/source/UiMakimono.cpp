#include "UiMakimono.h"
#include "playerbase.h"
#include "applicationglobal.h"

UiMakimono::UiMakimono():_player(nullptr)
{
	
}

UiMakimono::~UiMakimono()
{

}

bool UiMakimono::Initialize()
{
	base::Initialize();
	_handle = LoadGraph("res/Makimono.png");
	return true;
}

bool UiMakimono::Terminate()
{
	base::Terminate();
	if(_handle >= 0)
	{
		DeleteGraph(_handle);
		handle = -1;
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
	if(!_player)
	{
		return true;
	}

	// 画像サイズ取得（右下配置用）
	int w = 0;
	int h = 0;
	if(_handle >= 0)
	{
		GetGraphSize(_handle, &w, &h);
	}

	// 画面サイズ
	const int screenW = ApplicationBase::GetInstance()->DispSizeW();
	const int screenH = ApplicationBase::GetInstance()->DispSizeH();

	// 右下の余白（写真の感じに合わせて調整）
	const int marginX = 30;
	const int marginY = 30;

	// 右下基準で座標決定
	const int x = screenW - marginX - w;
	const int y = screenH - marginY - h;

	// 巻物画像を描画（透過あり）
	if(_handle >= 0)
	{
		DrawGraph(x, y, _handle, TRUE);
	}

	// 所持数（ひとまず文字。あとで数字画像に置換可能）
	{
		const int count = _player->GetMakimonoCount();
		const std::string text = "x" + std::to_string(count);

		// 画像の右側に少しずらして描画（ここも好みで）
		const int tx = x + w + 8;
		const int ty = y + (h / 2) - 8;
		DrawString(tx, ty, text.c_str(), GetColor(255, 255, 255));
	}
	return true;
}

