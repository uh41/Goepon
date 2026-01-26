/*********************************************************************/
// * \file   sounditemse.h
// * \brief  SEÄ¶ƒNƒ‰ƒX
// *
// * \author —é–Ø—T‹H
// * \date   2025/12/23
// * \ì‹Æ“à—e: V‹Kì¬ —é–Ø—T‹H@2025/12/23
/*********************************************************************/

#pragma once
#include "sounditemstatic.h"

namespace soundserver
{
	class SoundItemSE : public SoundItemStatic
	{
		typedef SoundItemStatic base;
	public:

		SoundItemSE(std::string filename, int flag = 0) : base(filename, flag)	{}

		virtual TYPE GetType()override { return TYPE::SE; }
		virtual void Play()override
		{
			this->Stop();
			PlayMem(DX_PLAYTYPE_BACK);
		}
	};
}

