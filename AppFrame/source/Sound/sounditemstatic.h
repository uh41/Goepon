/*********************************************************************/
// * \file   sounditemstaic.h
// * \brief  スタティック再生クラス
// *
// * \author 鈴木裕稀
// * \date   2025/12/23
// * \作業内容: 新規作成 鈴木裕稀　2025/12/23
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
