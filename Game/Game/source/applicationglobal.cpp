/*********************************************************************/
// * \file   applicationblocal.cpp
// * \brief  アプリケーショングローバルクラス
// *
// * \author 鈴木裕稀
// * \date   2025/12/15
// * \作業内容: 新規作成 鈴木裕稀　2025/12/15
/*********************************************************************/

#include "applicationglobal.h"
#include "appframe.h"


// 実体
ApplicationGlobal gGlobal;


// コンストラクタ
ApplicationGlobal::ApplicationGlobal()
{
	_iCgCursor = -1;
}

// デストラクタ
ApplicationGlobal::~ApplicationGlobal()
{
	//ResourceServer::Release();
	// 何もしない
}

// 初期化
bool ApplicationGlobal::Init()
{
	SetUseASyncLoadFlag(TRUE);
	_iCgCursor = LoadGraph("res/cursor.png");
	// キャラクター関連モデル読み込み
	ResourceServer::MV1LoadModel("res/Tanuhuman/TanuHuman.mv1");
	ResourceServer::MV1LoadModel("res/Tanuki/SK_goepon_multimotion.mv1");
	ResourceServer::MV1LoadModel("res/PoorEnemyMelee/bushi_0114taiki.mv1");
	// マップ関連モデル読み込み
	ResourceServer::MV1LoadModel("res/SkySphere/skysphere.mv1");
	ResourceServer::MV1LoadModel("res/Ground/Ground.mv1");
	ResourceServer::MV1LoadModel("res/map/SM_stagebeta.mv1");

	if(!_soundServer)
	{
		_soundServer = std::make_shared<soundserver::SoundServer>();
	}
	_soundServer->Add("bgminitialize", std::make_shared<soundserver::SoundItemBGM>(mp3::shinobiashi));
	_soundServer->Add("bgmChenge", std::make_shared<soundserver::SoundItemBGM>(wav::ks010));
	_soundServer->Add("1", std::make_shared<soundserver::SoundItemSE>(mp3::tanuki_walk));
	_soundServer->Add("2", std::make_shared<soundserver::SoundItemSE>(mp3::tanuki_henshin_start));
	_soundServer->Add("35", std::make_shared<soundserver::SoundItemSE>(mp3::bush_found));
	_soundServer->Add("31", std::make_shared<soundserver::SoundItemSE>(mp3::bushi_walk, soundserver::SoundItemBase::FLG_3D));
	_soundServer->Add("4", std::make_shared<soundserver::SoundItemSE>(mp3::tanuki_Tresure_open));
	_soundServer->Add("3", std::make_shared<soundserver::SoundItemSE>(mp3::tanuki_henshinfinish));
	_soundServer->Add("10", std::make_shared<soundserver::SoundItemSE>(mp3::tanubito_tailattack));
	_soundServer->Add("30", std::make_shared<soundserver::SoundItemSE>(mp3::bushi_stun_start));
	_soundServer->Add("doghowling", std::make_shared<soundserver::SoundItemSE>(mp3::dog_howling));
	_soundServer->Add("11", std::make_shared<soundserver::SoundItemSE>(mp3::tanubito_walk));
	_soundServer->Add("50", std::make_shared<soundserver::SoundItemSE>(mp3::okimono_move));
	_soundServer->Add("60", std::make_shared<soundserver::SoundItemSE>(mp3::UI_charin));
	_soundServer->Add("voice1", std::make_shared<soundserver::SoundItemVoice>(mp3::voice1, soundserver::SoundItemBase::FLG_3D));
	_soundServer->Add("voice2", std::make_shared<soundserver::SoundItemVoice>(mp3::voice2, soundserver::SoundItemBase::FLG_3D));
	_soundServer->Add("voice3", std::make_shared<soundserver::SoundItemVoice>(mp3::voice3, soundserver::SoundItemBase::FLG_3D));
	SetUseASyncLoadFlag(FALSE);

	return true;
}
