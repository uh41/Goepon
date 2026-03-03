#include "counterui.h"

CounterUi::CounterUi()
{
	_handleNum.fill(-1);
	Initialize();
}

bool CounterUi::Initialize()
{
	base::Initialize();
	const char* numPath[10] =
	{
		ui::UI_0,
		ui::UI_1,
		ui::UI_2,
		ui::UI_3,
		ui::UI_4,
		ui::UI_5,
		ui::UI_6,
		ui::UI_7,
		ui::UI_8,
		ui::UI_9
	};
	for (int i = 0; i < 10; i++)
	{
		_handleNum[i] = ResourceServer::LoadGraph(numPath[i]);
	}
	return true;
}

bool CounterUi::Terminate()
{
	for(int i = 0; i < 10; i++)
	{
		if (_handleNum[i] != -1)
		{
			DeleteGraph(_handleNum[i]);
			_handleNum[i] = -1;
		}
	}

	return true;
}

void CounterUi::RenderCount(int open, int total)
{
	int screenW = ApplicationBase::GetInstance()->DispSizeW();
	int screenH = ApplicationBase::GetInstance()->DispSizeH();
	// ”š‚Ì•`‰æ
	int numX = 10;
	int numY = 10;
	// ŠJ‚¢‚½”‚ğ•`‰æ
	if (open < 10)
	{
		DrawGraph(numX, numY, _handleNum[open], TRUE);
	}
	else
	{
		DrawGraph(numX, numY, _handleNum[1], TRUE);
		DrawGraph(numX + 30, numY, _handleNum[open % 10], TRUE);
	}
	// ‘”‚ğ•`‰æ
	if (total < 10)
	{
		DrawGraph(numX + 60, numY, _handleNum[total], TRUE);
	}
	else
	{
		DrawGraph(numX + 60, numY, _handleNum[1], TRUE);
		DrawGraph(numX + 90, numY, _handleNum[total % 10], TRUE);
	}
}

bool CounterUi::Process()
{
	base::Process();
	return true;
}

bool CounterUi::Render()
{
	base::Render();
	return true;
}