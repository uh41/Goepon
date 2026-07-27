/*********************************************************************/
// * \file   sounditemvoice.h
// * \brief  ボイスクラス
// *
// * \author 鈴木裕稀
/*********************************************************************/

#pragma once
#include "sounditemstream.h"

namespace soundserver
{
	class SoundItemVoice : public SoundItemStream
	{
		typedef SoundItemStream base;
	public:
		SoundItemVoice(std::string filename, int flag = 0) : base(filename, flag)	{}

		virtual TYPE GetType()override { return TYPE::VOICE; }

		virtual void Play()override
		{
			StreamLoad();	// ストリーム再生のものをロードする
			if(IsPlay() == false)
			{
				PlayMem(DX_PLAYTYPE_BACK);		// 1回のみ再生で再生する
			}
		}
	};
}
