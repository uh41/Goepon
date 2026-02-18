/*********************************************************************/
// * \file   sounditemse.h
// * \brief  SE再生クラス
// *
// * \author 鈴木裕稀
// * \date   2025/12/23
// * \作業内容: 修正 鈴木裕稀 2026/02/16
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
			// 未ロードならロードしてから再生する（同期ロード）
			if(_iSoundHandle == -1)
			{
				_iSoundHandle = LoadMem(_sFileName);
				// ロード失敗なら何もしない
				if(_iSoundHandle == -1)
				{
					return;
				}
			}

			this->Stop();
			int playType;
			// ループフラグが立っているかで再生タイプを切り替える
			if(_iFlag & FLG_LOOP)
			{
				playType = DX_PLAYTYPE_LOOP;
			}
			else
			{
				playType = DX_PLAYTYPE_BACK;
			}
			PlayMem(playType);
		}
	};
}