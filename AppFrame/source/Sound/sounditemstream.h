/*********************************************************************/
// * \file   sounditemstream.h
// * \brief  ストリーム再生クラス
// *
// * \author 鈴木裕稀
/*********************************************************************/

#pragma once
#include "sounditembase.h"

namespace soundserver
{
	class SoundItemStream : public SoundItemBase
	{
		typedef SoundItemBase base;
	public:
		SoundItemStream(std::string filename, int flag) : base(filename, flag)	{}

		virtual ~SoundItemStream() {}

		virtual bool IsLoad() override { return true; }		// ストリーム再生のものは、ロードされているものとする

	protected:
		// ストリーム再生のものは、ロードされているものとする
		void StreamLoad()
		{
			if(_iSoundHandle == -1)
			{
				SetCreateSoundDataType(DX_SOUNDDATATYPE_FILE);
				_iSoundHandle = LoadMem(_sFileName);

				if(_iFrequency == 0)
				{
					_iFrequency = GetFrequencySoundMem(_iSoundHandle);
				}
			}
		}

		// ストリーム再生のものは、再生していなかったらメモリからアンロードする
		virtual void Update()
		{
			if(_iSoundHandle != -1)
			{
				if(IsPlay() == false)
				{
					Unload();	// ストリーム再生のものは、再生していなかったらメモリからアンロードする
				}
			}
		}
	};
}
