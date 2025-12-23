/*********************************************************************/
// * \file   sounditembase.h
// * \brief  サウンドベースクラス
// *
// * \author 鈴木裕稀
// * \date   2025/12/23
// * \作業内容: 新規作成 鈴木裕稀　2025/12/23
/*********************************************************************/

#pragma once
#include "../container.h"

class SoundServer;

// サウンドアイテム基底クラス
namespace soundserver
{
	class SoundItemBase
	{
	public:
		enum class TYPE
		{
			BGM = 0,		// ストリーミング再生。ループ再生
			SE,				// 1回のみ再生
			VOICE,			// ストリーミング再生。ループ再生しない
			ONESHOT,		// ストリーミング再生。1回のみ再生。即時再生し、停止したらサーバから削除される
		};
		static const int FLG_3D = 0x0001;			// 3Dサウンドとして再生する

		SoundItemBase(std::string filename, int flg);
		virtual ~SoundItemBase();

		void SetSoundServer(SoundServer* soundserver) { _soundServer = soundserver; }

		virtual void Unload();						// サウンドのアンロード
		virtual bool IsLoad();						// サウンドがロードされているか？
		virtual void Play() = 0;					// サウンドの再生
		virtual bool IsPlay();						// サウンドが再生中か？
		virtual void Stop();						// サウンドの停止

		virtual int GetVolume();					// ボリューム取得
		virtual int GetPan();						// パン取得
		virtual int GetFrequency();					// 周波数取得
		virtual void SetVolume(int volume);			// ボリューム設定
		virtual void SetPan(int pan);				// パン設定
		virtual void SetFrequency(int frequency);	// 周波数設定
		virtual void ResetFrequency();				// 周波数リセット

		virtual TYPE GetType() = 0;					// サウンドタイプ取得
		virtual std::string GetFileName() { return _sFileName; } // ファイル名取得
		virtual int GetSoundHandle() { return _iSoundHandle; } // サウンドハンドル取得

		virtual void Updata() {};					// サウンドの更新

	protected:
		virtual void  PlayMem(int flat);			// メモリからサウンド再生
		virtual int LoadMem(std::string filename);						// メモリからサウンドロード


		int	_iSoundHandle; // サウンドハンドル
		std::string _sFileName; // ファイル名
		int	_iFlag;	 // フラグ
		int	_iPan;	 // パン
		int _iFrequency; // 周波数
		SoundServer* _soundServer; // サウンドサーバー
	};
}



