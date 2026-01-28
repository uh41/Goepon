#include "effekseermanager.h"

EffekseerManager* EffekseerManager::GetInstance()
{
	static EffekseerManager instance;
	return &instance;
}

bool EffekseerManager::Initialize()
{
	if(_initialize)
	{
		return true;
	}

	if(Effekseer_Init(20000) != 0)
	{
		return false;
	}

	_initialize = true;
	return true;
}

bool EffekseerManager::Terminate()
{
	if(!_initialize)
	{
		return false;
	}
	DeleteAllEffect();

	Effkseer_End();

	_initialize = false;

	return true;
}

int EffekseerManager::LoadEffect(const char* path, float scale)
{
	if(!path)
	{
		return -1;
	}

	auto it = _effect.find(std::string(path));
	if(it != _effect.end())
	{
		return it->second;
	}

	// エフェクト読み込み
	int handle = LoadEffekseerEffect(path, scale);
	if(handle != -1)
	{
		_effect.emplace(std::string(path),handle);
	}
	return handle;
}

bool EffekseerManager::DeleteEffect(int handle)
{
	if(handle == -1)
	{
		return false;
	}

	DeleteEffekseerEffect(handle);

	for(auto it = _effect.begin(); it != _effect.end(); it++)
	{
		if(it->second == handle)
		{
			it = _effect.erase(it);
		}
		else
		{
			++it;
		}
	}

	_effectSpeed.erase(handle);

	return true;
}

bool EffekseerManager::UnloadEffect(const char* path)
{
	if(!path)
	{
		return false;
	}

	auto it = _effect.find(std::string(path));
	if(it == _effect.end())
	{
		return false;
	}

	int handle = it->second;
	DeleteEffekseerEffect(handle);

	_effect.erase(it);
	_effectSpeed.erase(handle);

	return true;
}

void EffekseerManager::DeleteAllEffect()
{
	for(auto& e : _effect)
	{
		DeleteEffekseerEffect(e.second);
	}
	_effect.clear();
	_effectSpeed.clear();
}

bool EffekseerManager::IsLoadedEffect(const char* path) const
{
	if(!path)
	{
		return false;
	}
	auto it = _effect.find(std::string(path));
	if(it == _effect.end())
	{
		return false;
	}

	return true;
}

int EffekseerManager::PlayEffect3D(int handle)
{
	if(handle == -1)
	{
		return -1;
	}
	
	int playing = PlayEffekseer3DEffect(handle);
	if(playing != -1)
	{
		float speed = PlayEffekseer3DEffect(handle);
		_effectSpeed.emplace(playing, speed);
	}

	return playing;
}

int EffekseerManager::PlayEffect3DPos(int handle, const vec::Vec3& pos)
{
	if(handle == -1)
	{
		return -1;
	}
	int playing = PlayEffekseer3DEffect(handle);
	if(playing != -1)
	{
		SetPosPlayingEffekseer3DEffect(playing, pos.x, pos.y, pos.z);
		float speed = PlayEffekseer3DEffect(handle);
		_effectSpeed.emplace(playing, speed);
	}
	return playing;
}

bool EffekseerManager::StopEffect(int handle)
{
	if(handle == -1)
	{
		return false;
	}
	int res = StopEffekseer3DEffect(handle);
	if(res == -1)
	{
		return false;
	}
	_effectSpeed.erase(handle);
	return true;
}

bool EffekseerManager::IsPlayingEffect(int handle) const
{
	if(handle == -1)
	{
		return false;
	}
	int res = IsEffekseer3DEffectPlaying(handle);
	if(res == -1)
	{
		return false;
	}
	return true;
}

bool EffekseerManager::SetPauseEffect(int handle, bool pause)
{
	if(handle == -1)
	{
		return false;
	}
	if(pause)
	{
		// 停止
		int res = StopEffekseer3DEffect(handle);
		if(res == -1)
		{
			return false;
		}
	}
	else
	{
		// 再開
		int playing = PlayEffekseer3DEffect(handle);
		if(playing != -1)
		{
			auto it = _effectSpeed.find(handle);
			if(it != _effectSpeed.end())
			{
				float speed = it->second;
				_effectSpeed.emplace(playing, speed);
			}
		}
	}
	return true;
}

bool EffekseerManager::SetRotationEffect(int handle, const vec::Vec3& rad)
{
	if(handle == -1)
	{
		return false;
	}
	//// Effekseer側でラジアン指定が無いため、度に変換
	//float degX = mymath::RAD2DEG(rad.x);
	//float degY = mymath::RAD2DEG(rad.y);
	//float degZ = vec::RadToDeg(rad.z);
	//int res = SetRotationPlayingEffekseer3DEffect(handle, degX, degY, degZ);
	//if(res == -1)
	//{
	//	return false;
	//}
	return true;
}