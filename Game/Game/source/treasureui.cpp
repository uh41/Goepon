#include "treasureui.h"
#include "applicationmain.h"

TreasureUi::TreasureUi()
{
	_handleNum.fill(-1);
	Initialize();
}

bool TreasureUi::Initialize()
{
	base::Initialize();
	_handleDeguti = ResourceServer::LoadGraph(ui::Deguti);
	_handleNokori = ResourceServer::LoadGraph(ui::UI_nokori);
	_handleKakeru = ResourceServer::LoadGraph(ui::UI_x);

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
	_totalTreasure = 0;
	_openTreasure = 0;
	return true;
}

bool TreasureUi::Terminate()
{
	if (_handleDeguti != -1)
	{
		DeleteGraph(_handleDeguti);
		_handleDeguti = -1;
	}

	if (_handleNokori != -1)
	{
		DeleteGraph(_handleNokori);
		_handleNokori = -1;
	}

	if (_handleKakeru != -1)
	{
		DeleteGraph(_handleKakeru);
		_handleKakeru = -1;
	}

	for (int i = 0; i < 10; i++)
	{
		if (_handleNum[i] != -1)
		{
			DeleteGraph(_handleNum[i]);
			_handleNum[i] = -1;
		}
	}

	return true;
}

bool TreasureUi::Process()
{
	base::Process();

	_totalTreasure = static_cast<int>(_treasure.size());
	_openTreasure = 0;

	for (const auto& t : _treasure)
	{
		if (t && t->IsOpen())
		{
			_openTreasure++;
		}
	}

	return true;
}

bool TreasureUi::Render()
{
	base::Render();
	if (_handleDeguti == -1 || _handleNokori == -1 || _handleKakeru == -1)
	{
		return false;
	}

	int screenW = ApplicationBase::GetInstance()->DispSizeW();
	int screenH = ApplicationBase::GetInstance()->DispSizeH();


	DrawGraph(10, 10, _handleDeguti, TRUE);
	DrawGraph(10, 50, _handleNokori, TRUE);
	DrawGraph(60, 50, _handleKakeru, TRUE);
	// ”š‚Ì•`‰æ
	int numX = 100;
	int numY = 50;
	// ŠJ‚¢‚½•ó” ‚Ì”‚ğ•`‰æ
	if (_openTreasure < 10)
	{
		DrawGraph(numX, numY, _handleNum[_openTreasure], TRUE);
	}
	else
	{
		DrawGraph(numX, numY, _handleNum[1], TRUE);
		DrawGraph(numX + 30, numY, _handleNum[_openTreasure % 10], TRUE);
	}
	// ‘”‚ğ•`‰æ
	if (_totalTreasure < 10)
	{
		DrawGraph(numX + 60, numY, _handleNum[_totalTreasure], TRUE);
	}
	else
	{
		DrawGraph(numX + 60, numY, _handleNum[1], TRUE);
		DrawGraph(numX + 90, numY, _handleNum[_totalTreasure % 10], TRUE);
	}
	return true;
}