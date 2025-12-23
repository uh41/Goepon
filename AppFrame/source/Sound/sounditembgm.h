/*********************************************************************/
// * \file   sounditembgm.h
// * \brief  BGM再生クラス
// *
// * \author 鈴木裕稀
// * \date   2025/12/23
// * \作業内容: 新規作成 鈴木裕稀　2025/12/23
/*********************************************************************/

#pragma once
#include "sounditemstream.h"
#include "soundserver.h"

class SoundServer;

// BGM再生クラス
namespace soundserver
{
	class SoundItemBGM : public SoundItemStream
	{
		typedef SoundItemStream base;
	public:
		SoundItemBGM(std::string filename, int flag = 0) : base(filename, flag)	{}
		virtual TYPE GetType()override { return TYPE::BGM; }
		virtual void Play()override
		{
			if(_soundServer)
			{
				_soundServer->StopType(TYPE::BGM); // 他のBGMを停止する
			}

			StreamLoad();	// ストリーム再生のものをロードする

			if(IsPlay() == false)
			{
				PlayMem(DX_PLAYTYPE_LOOP);		// ループ再生で再生する
			}
		}
	};
}