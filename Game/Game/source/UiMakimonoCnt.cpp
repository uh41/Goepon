
#include "UiMakimonoCnt.h"
#include "playerbase.h"
#include "applicationglobal.h"
#include "appframe.h"

UiMakimonoCnt::UiMakimonoCnt()
{
	_player = nullptr;
	_handleMakimono = -1;
	_handleUiX = -1;
}

UiMakimonoCnt::~UiMakimonoCnt()
{

}

bool UiMakimonoCnt::Initialize()
{
	base::Initialize();

	// 数字ハンドルの読み込む
	const char* makiNumPath[10] =
	{
		ui::MUI_0,
		ui::MUI_1,
		ui::MUI_2,
		ui::MUI_3,
		ui::MUI_4,
		ui::MUI_5,
		ui::MUI_6,
		ui::MUI_7,
		ui::MUI_8,
		ui::MUI_9
	};
	for(int i = 0; i < 10; i++)
	{
		// 既に読み込まれているハンドル（UI_0..UI_9）を解放して置き換え
		if(_handleNum[i] != -1)
		{
			DeleteGraph(_handleNum[i]);
			_handleNum[i] = -1;
		}
		_handleNum[i] = LoadGraph(makiNumPath[i]);
	}

	return true;
}

bool UiMakimonoCnt::Terminate()
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

bool UiMakimonoCnt::Process()
{
	base::Process();
	return true;
}

bool UiMakimonoCnt::Render()
{
	base::Render();

	// 復帰直後などで _player が未設定の可能性がある
	if(_player == nullptr)
	{
		return true;
	}

	// 巻物の所有数を描画
	RenderNumber(_player->GetMakimonoCount(), makimono::MAKIMONO_COUNT_X, makimono::MAKIMONO_COUNT_Y, true);

	return true;
}


