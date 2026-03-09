#include "moviemanager.h"

MovieManager* MovieManager::GetInstance()
{
	static MovieManager instance;
	return &instance;
}

MovieManager::MovieManager()
{
	_initialized = false;
}

bool MovieManager::Initialize()
{
	_initialized = true;
	_handle.clear();
	return true;
}

bool MovieManager::Terminate()
{
	for(int handle : _handle)
	{
		DeleteGraph(handle);
	}

	_handle.clear();
	_initialized = false;
	return true;
}

int MovieManager::LoadMovie(const std::string& path)
{
	int handle = OpenMovieToGraph(path.c_str(), FALSE);
	if(handle != -1)
	{
		_handle.insert(handle);// 成功したらハンドルをセットに追加
	}
	return handle;
}

bool MovieManager::PlayMovie(int handle, bool loop)
{
	if(handle < 0)
	{
		return false;
	}

	int loopFlag;
	if(loop)
	{
		loopFlag = true;
	}
	else
	{
		loopFlag = false;
	}

	int ret = PlayMovieToGraph(handle, loopFlag);

	return ret == 0; // 成功したら0が返る
}

bool MovieManager::StopMovie(int handle)
{
	if(handle < 0)
	{
		return false;
	}

	PauseMovieToGraph(handle); // 再生を停止する

	return true;
}

void MovieManager::UnloadMovie(int handle)
{
	if(handle < 0)
	{
		return;
	}
	auto it = _handle.find(handle);
	if(it != _handle.end())
	{
		if(IsMoviePlaying(handle))
		{
			StopMovie(handle);
		}

		DeleteGraph(handle);
		_handle.erase(it);
	}
}

bool MovieManager::IsMoviePlaying(int handle)
{
	if(handle < 0) return false;

	// DxLib のムービー状態を問い合わせるAPIを利用して再生中か確認する
	// 環境によって定数が異なる場合はここを調整してください。
	int state = GetMovieStateToGraph(handle); // DxLib API
	// 一般的に MOVIE_STATE_PLAY 相当の値は 1 のことが多いです。
	const int MOVIE_STATE_PLAY = 1;
	return (state == MOVIE_STATE_PLAY);
}