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
	int handle = LoadGraph(path.c_str());
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
		loopFlag = TRUE;
	}
	else
	{
		loopFlag = FALSE;
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

#ifdef StopMovieToGraph
	StopMovieToGraph(handle);
#else
	StopMovie(handle);
#endif

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
		DeleteGraph(handle);
		_handle.erase(it);
	}
}