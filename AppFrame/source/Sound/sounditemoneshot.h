/*********************************************************************/
// * \file   sounditemoneshot.h
// * \brief  OneShot再生クラス
// *
// * \author 鈴木裕稀
// * \date   2025/12/23
// * \作業内容: 新規作成 鈴木裕稀　2025/12/23
/*********************************************************************/

#pragma once
#include "sounditemstream.h"
#include "soundserver.h"

namespace soundserver
{
	class SoundItemOneShot : public SoundItemStream
	{
		typedef SoundItemStream base;
	public:
		SoundItemOneShot(std::string filename, int flag = 0) : base(filename, flag) {}
		virtual TYPE GetType()override { return TYPE::ONESHOT; }
		virtual void Play()override
		{
			StreamLoad();	// ストリーム再生のものをロードする
			PlayMem(DX_PLAYTYPE_BACK);		// 1回のみ再生で再生する
		}

		virtual void Update() override
		{
			int sound = _iSoundHandle;
			base::Update();
			if(sound != -1 && _iSoundHandle == -1)
			{
				// 再生されていたがUnload()されている場合、自分を削除
				_soundServer->Del(this);
			}
		}
	};
}