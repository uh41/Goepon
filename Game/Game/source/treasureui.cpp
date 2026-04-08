#include "treasureui.h"
#include "applicationmain.h"
#include "modegame.h"

TreasureUi::TreasureUi()
{
	_handleDeguti = -1;
	_handleNokori = -1;
	_handleGoDeguti = -1;

	_remainCount = 0;
	_treasureCount = 0;
	_isTreasureListAssigned = false;

	_isGoDegutiShowActive = false;
	_goDegutiShowElapsed  = 0.0f;
	_ownerGame = nullptr;
}

bool TreasureUi::Initialize()
{
	base::Initialize();
	_handleDeguti = LoadGraph(ui::Deguti);
	_handleNokori = LoadGraph(ui::UI_nokori);
	_handleGoDeguti = LoadGraph(ui::Gotoexit);

	_remainCount = 0;
	_treasureCount = 0;
	_isTreasureListAssigned = false;

	_isGoDegutiShowActive = false;
	_goDegutiShowElapsed  = 0.0f;
	return true;
}

bool TreasureUi::Terminate()
{
	base::Terminate();

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

	if(_handleGoDeguti != -1)
	{
		DeleteGraph(_handleGoDeguti);
		_handleGoDeguti = -1;
	}

	return true;
}

bool TreasureUi::Process()
{
	base::Process();

	// お宝リストがセットされていない場合はカウントを0にしてUIも非表示にする
	if(!_isTreasureListAssigned)
	{
		_remainCount		  = 0;
		_treasureCount		  = 0;
		_isGoDegutiShowActive = false;
		_goDegutiShowElapsed  = 0.0f;
		return true;
	}

	const int oldRemainCount = _remainCount; // お宝の残り数を更新するために古い値を保存

	int remainCount   = 0;	
	int treasureCount = 0;

	// 残りの数と総数をカウントする
	for(const auto& treasure : _treasure)
	{
		if(!treasure)
		{
			continue;
		}

		treasureCount++;

		if(!treasure->IsOpen())
		{
			remainCount++;
		}
	}

	_remainCount   = remainCount;	
	_treasureCount = treasureCount;

	// お宝をすべて獲得した直後だけ巻物UIを一定時間表示
	if(oldRemainCount > 0 && _remainCount == 0 && _treasureCount > 0)
	{
		_isGoDegutiShowActive = true;
		_goDegutiShowElapsed  = 0.0f;
	}

	// 巻物UIの表示時間制御
	if(_isGoDegutiShowActive)
	{
		_goDegutiShowElapsed += 1.0 / 60.0f; 
		
		// 表示時間が経過したらフラグを立てる
		if(_goDegutiShowElapsed >= treasure::GO_DEGUTI_DURATION)
		{
			_isGoDegutiShowActive = false;
		}
		
		if(_remainCount > 0)
		{
			_isGoDegutiShowActive = false;
		}
	}
	return true;
}

bool TreasureUi::Render()
{
	base::Render();

	if(_handleDeguti == -1 || _handleNokori == -1 || _handleGoDeguti == -1)
	{
		return false;
	}

	// 全て取れていないときにUIを表示
	if(_remainCount > 0)
	{
		DrawGraph(treasure::NOKORI_X, treasure::NOKORI_Y, _handleNokori, TRUE);
		RenderNumber(_remainCount, counter::COUNTER_TREASURE_X, counter::COUNTER_TREASURE_Y, false);
		return true;
	}

	// 全て取ったときにUIの切り替え
	if(_handleDeguti != -1)
	{
		DrawGraph(treasure::DEGUTI_X, treasure::DEGUTI_Y, _handleDeguti, TRUE);
	}

	// 全部取った直後だけメッセージを一定時間表示
	if(_isGoDegutiShowActive && _handleGoDeguti != -1)
	{
		DrawGraph(treasure::GOEXIT_X, treasure::GOEXIT_Y, _handleGoDeguti, TRUE);
	}
	return true;
}