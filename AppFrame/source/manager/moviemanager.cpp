/*********************************************************************/
// * \file   moviemanager.cpp
// * \brief  ムービー管理クラス
// *
// * \author 鈴木裕稀
/*********************************************************************/

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
		loopFlag = 1;
	}
	else
	{
		loopFlag = 0;
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

	// ハンドルがセットに存在するか確認
	auto it = _handle.find(handle);
	if(it != _handle.end())
	{
		// ムービーが再生中であれば停止する
		if(!IsMoviePlaying(handle))
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

	int state = GetMovieStateToGraph(handle);// 動画の再生状態を取得
	return (state == movie::MOVIE_STATE_PLAY);
}