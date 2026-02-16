/*********************************************************************/
// * \file   sounditemse.h
// * \brief  SEÄ¶ƒNƒ‰ƒX
// *
// * \author —é–Ø—T‹H
// * \date   2025/12/23
// * \ì‹Æ“à—e: C³ —é–Ø—T‹H 2026/02/16
/*********************************************************************/

#pragma once
#include "sounditemstatic.h"

namespace soundserver
{
	class SoundItemSE : public SoundItemStatic
	{
		typedef SoundItemStatic base;
	public:

		SoundItemSE(std::string filename, int flag = 0) : base(filename, flag) {}

		virtual TYPE GetType()override { return TYPE::SE; }

		virtual void Play()override
		{
			// –¢ƒ[ƒh‚È‚çƒ[ƒh‚µ‚Ä‚©‚çÄ¶‚·‚éi“¯Šúƒ[ƒhj
			if(_iSoundHandle == -1)
			{
				_iSoundHandle = LoadMem(_sFileName);
				// ƒ[ƒhŽ¸”s‚È‚ç‰½‚à‚µ‚È‚¢
				if(_iSoundHandle == -1)
				{
					return;
				}
			}

			this->Stop();
			PlayMem(DX_PLAYTYPE_BACK);
		}
	};
}