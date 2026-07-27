/*********************************************************************/
// * \file   sounditemstatic.h
// * \brief  スタティック再生クラス
// *
// * \author 鈴木裕稀
/*********************************************************************/

#pragma once
#include "sounditembase.h"
#include "../container.h"

// スタティック再生クラス
namespace soundserver
{
	class SoundItemStatic : public SoundItemBase
	{
		typedef SoundItemBase base;
	public:
		SoundItemStatic(std::string filename, int flag) : base(filename, flag)
		{
			SetCreateSoundDataType(DX_SOUNDDATATYPE_MEMNOPRESS);
		}
	};
}
