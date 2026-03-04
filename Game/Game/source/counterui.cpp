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

void CounterUi::DecreaseTreasureCount()
{
	if(_treasureCount > 0)
	{
		_treasureCount--;
	}
}

bool CounterUi::Process()
{
	base::Process();
	return true;
}

void CounterUi::RenderNumber(int number,int x, int y, bool drawZero)
{
	if(number < 0 || number >99)
	{
		return;
	}

	if(!drawZero && number == 0)
	{
		return;
	}

	if(number < 10)
	{
		DrawGraph(x, y, _handleNum[number], TRUE);
	}
	else
	{
		int tens = number / 10; // 10‚ÌˆÊ
		int ones = number % 10; // 1‚ÌˆÊ
		DrawGraph(x, y, _handleNum[tens], TRUE);
		DrawGraph(x + counter::COUNTER_TREASURE_NEXT_X, y, _handleNum[ones], TRUE);
	}
}

bool CounterUi::Render()
{
	base::Render();

	RenderNumber(_treasureCount, counter::COUNTER_TREASURE_X, counter::COUNTER_TREASURE_Y);

	return true;
}