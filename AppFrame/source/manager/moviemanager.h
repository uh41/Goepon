#pragma once

#include "../container.h"
#include "../aliastemplate.h"

class MovieManager
{
public:
	static MovieManager* GetInstance();

	bool Initialize();
	bool Terminate();

	// 動画の読み込む
	int LoadMovie(const std::string& path);

	// 動画の再生
	bool PlayMovie(int handle, bool loop = false);

	// 動画の停止
	bool StopMovie(int handle);

	// 動画の解放
	void UnloadMovie(int handle);

	// 動画が再生中かどうかを確認
	bool IsMoviePlaying(int handle);

private:
	MovieManager();
	~MovieManager() = default;

	at::ust<int> _handle;// 動画ハンドルのセット

	bool _initialized; // 初期化フラグ

};

