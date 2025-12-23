/*********************************************************************/
// * \file   soundserver.cpp
// * \brief  サウンドサーバークラス
// *
// * \author 鈴木裕稀
// * \date   2025/12/23
// * \作業内容: 新規作成 鈴木裕稀　2025/12/23
/*********************************************************************/


#include "soundserver.h"
#include "sounditembase.h"

// コンストラクタ
soundserver::SoundServer::SoundServer()
{
		_bIsUpdate = false;
		_iCntOneShot = 0;
}

// デストラクタ
soundserver::SoundServer::~SoundServer()
{
	Clear();
}

// サウンドサーバーのクリア
void soundserver::SoundServer::Clear()
{
	for(auto&& e : _v)
	{
		delete e.second;
	}
	_v.clear();
}

void soundserver::SoundServer::Add(SoundItemOneShot* oneshot)
{

}
