/*********************************************************************/
// * \file   soundserver.h
// * \brief  サウンドサーバークラス
// *
// * \author 鈴木裕稀
// * \date   2025/12/23
// * \作業内容: 新規作成 鈴木裕稀　2025/12/23
/*********************************************************************/

#pragma once
#include "../container.h"
#include "../aliastemplate.h"
#include "sounditembase.h"

// 前方宣言
namespace soundserver 
{
	class SoundItemOneShot;
}

// サウンドサーバークラス
namespace soundserver
{
	class SoundServer
	{
	public:
		SoundServer();
		virtual ~SoundServer();

		void Clear();
		void Add(SoundItemOneShot* oneshot);
		void Add(std::string name, SoundItemBase* sound);
		bool Del(std::string name);
		bool Del(SoundItemBase* sound);
		SoundItemBase* Get(std::string name);

		at::msc<SoundItemBase> GetVector() { return _v; }

		void StopType(SoundItemBase::TYPE type);

		void Update();

	protected:
		bool _bIsUpdate; // 更新中フラグ
		at::msc<SoundItemBase> _v;		// サウンドアイテムコンテナ
		at::msc<SoundItemBase> _vAdd;	// 追加用コンテナ
		at::msc<SoundItemBase> _vDel;	// 削除用コンテナ

		int _iCntOneShot; // OneShot用カウント
	};
}

