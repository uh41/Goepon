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
	_soundServer = std::make_shared<soundserver::SoundServer>();
	_voice = nullptr;

	return true;
}

bool ModeScenarioBase::Terminate()
{
	_soundServer->Clear();
	//_soundServer = nullptr;
	//_voice = nullptr;
	//_page.clear();
	//_pageNo = 0;

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
		static_cast<int>(_page.size()) - 1,
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
		_voice = nullptr;

		if(!_page[_pageNo].voiceFile.empty())		// ボイスファイルがある場合
		{
			_voice = new soundserver::SoundItemVoice(_page[_pageNo].voiceFile);// ボイス作成
			_soundServer->Add("voice", _voice);// サウンドサーバーに追加
			_voice->Play();// ボイス再生
		}
	}
}