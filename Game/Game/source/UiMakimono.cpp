#include "UiMakimono.h"
#include "playerbase.h"
#include "applicationglobal.h"
#include "appframe.h"

UiMakimono::UiMakimono()
{
	_player = nullptr;
	_handleMakimono = -1;
	_handleUiX = -1;
}

UiMakimono::~UiMakimono()
{

}

bool UiMakimono::Initialize()
{
	base::Initialize();
	_handle = LoadGraph("res/Makimono/Makimono (1).png");

	//_handleMakimono = LoadGraph(ui::Item_Makimono);
	//_handleUiX = LoadGraph(ui::UI_x);
	return true;
}

bool UiMakimono::Terminate()
{
	base::Terminate();
	if(_handleMakimono)
	{
		DeleteGraph(_handleMakimono);
		_handleMakimono = -1;
	}
	if(_handleUiX)
	{
		DeleteGraph(_handleUiX);
		_handleUiX = -1;
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

	//// 画像が未ロードなら描画しない
	//if(_handleMakimono == -1 || _handleUiX == -1 || _handle == -1)
	//{
	//	return false;
	//}

	DrawGraph(makimono::MAKIMONO_X, makimono::MAKIMONO_Y, _handleMakimono, TRUE);
	DrawGraph(makimono::KAKERU_X, makimono::KAKERU_Y, _handleUiX, TRUE);

	// 復帰直後などで _player が未設定の可能性がある
	if(_player == nullptr)
	{
		return true;
	}

	RenderNumber(_player->GetMakimonoCount(), makimono::MAKIMONO_COUNT_X, makimono::MAKIMONO_COUNT_Y, true);

	return true;
}


