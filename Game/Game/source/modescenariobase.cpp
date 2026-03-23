#include "modescenariobase.h"
#include "mymath.h"

ModeScenarioBase::ModeScenarioBase()
{
	Initialize();
}

ModeScenarioBase::~ModeScenarioBase()
{
	//Terminate();
}

bool ModeScenarioBase::Initialize()
{
	_page.clear();
	_pageNo = 0;
	_backHandle = -1;
	_soundServer = std::make_shared<soundserver::SoundServer>();
	_se = nullptr;
	_moveHandle = -1;

	return true;
}

bool ModeScenarioBase::Terminate()
{
	if (_soundServer)
	{
		_soundServer->Clear();
		_soundServer = nullptr;  // nullptrにセット
	}
	_se = nullptr;  // nullptrにセット

	// ページの画像リソースを解放
	for (auto& page : _page)
	{
		if (page.handle != -1)
		{
			DeleteGraph(page.handle);
			page.handle = -1;
		}
	}
	_page.clear();
	_pageNo = 0;
	_backHandle = -1;

	return true;
}

bool ModeScenarioBase::Process()
{
	// サウンド更新
	_soundServer->Update();
	return true;
}

bool ModeScenarioBase::Render()
{
	return true;
}

void ModeScenarioBase::ChangePage(int pageNo)
{
	if(_page.empty())
	{
		return;
	}

	int oldPageNo = _pageNo;
	int nextpage = mymath::Clamp(
		0,
		StCas<int>(_page.size()) - 1,
		oldPageNo + pageNo
	);// 範囲内に収める

	if(nextpage == oldPageNo)
	{
		return;
	}

	_pageNo = nextpage;

	// ボイス再生
	if(_soundServer)
	{
		_soundServer->StopType(soundserver::SoundItemBase::TYPE::VOICE);
		_se = nullptr;

		if(!_page[_pageNo].voiceFile.empty())		// ボイスファイルがある場合
		{
			_se = std::make_shared<soundserver::SoundItemSE>(_page[_pageNo].voiceFile);// ボイス作成
			_soundServer->Add("se", _se);// サウンドサーバーに追加
			_se->Play();// ボイス再生
		}
	}
}