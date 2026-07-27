/*********************************************************************/
// * \file   soundserver.h
// * \brief  サウンドサーバークラス
// *
// * \author 鈴木裕稀
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

		void Clear();													// クリア
		void Add(SoundItemOneShot* oneshot);							// OneShot専用の追加
		void Add(std::string name, at::spc<SoundItemBase> sound);		// 名前付きで追加
		bool Del(std::string name);										// 名前付きで削除
		bool Del(SoundItemBase* sound);									// サウンドを削除
		SoundItemBase* Get(std::string name);							// 名前付きで取得

		at::msc<at::spc<SoundItemBase>> GetVector() { return _v; }		// サウンドを取得

		void StopType(SoundItemBase::TYPE type);						// 指定のタイプのサウンドを停止する

		void Update();													// サウンドの更新

	protected:
		bool _bIsUpdate;						// 更新中フラグ
		at::msc<at::spc<SoundItemBase>> _v;		// サウンドアイテムコンテナ
		at::msc<at::spc<SoundItemBase>> _vAdd;	// 追加用コンテナ
		at::msc<at::spc<SoundItemBase>> _vDel;	// 削除用コンテナ

		int _iCntOneShot;						// OneShot用カウント
	};
}

