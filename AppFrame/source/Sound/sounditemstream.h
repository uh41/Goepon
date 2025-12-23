/*********************************************************************/
// * \file   sounditemstream.h
// * \brief  ストリーム再生クラス
// *
// * \author 鈴木裕稀
// * \date   2025/12/23
// * \作業内容: 新規作成 鈴木裕稀　2025/12/23
/*********************************************************************/

#pragma once
#include "sounditembase.h"

// ストリーミング再生クラス
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
