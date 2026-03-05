#include "treasureui.h"
#include "applicationmain.h"

TreasureUi::TreasureUi()
{

}

bool TreasureUi::Initialize()
{
	base::Initialize();
	_handleDeguti = LoadGraph(ui::Deguti);
	_handleNokori = LoadGraph(ui::UI_nokori);
	_handleMakimono = LoadGraph(ui::Makimono);

	_remainCount = 0;

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

	if(_handleMakimono != -1)
	{
		DeleteGraph(_handleMakimono);
		_handleMakimono = -1;
	}

	return true;
}

bool TreasureUi::Process()
{
	base::Process();

	_remainCount = 0;

	for(const auto& treasure : _treasure)
	{
		if(treasure && !treasure->IsOpen())
		{
			_remainCount++;
		}
	}

	return true;
}

bool TreasureUi::Render()
{
	base::Render();

	if(_handleDeguti == -1 || _handleNokori == -1 || _handleMakimono == -1)
	{
		return false;
	}

	DrawGraph(treasure::MAKIMONO_X, treasure::MAKIMONO_Y, _handleMakimono, TRUE);

	if(_remainCount > 0)
	{
		DrawGraph(treasure::NOKORI_X, treasure::KAKERU_Y, _handleNokori, TRUE);
		RenderNumber(_remainCount, counter::COUNTER_TREASURE_X, counter::COUNTER_TREASURE_Y, false);
	}
	else
	{
		DrawGraph(treasure::DEGUTI_X, treasure::DEGUTI_Y, _handleDeguti, TRUE);
	}

	return true;
}