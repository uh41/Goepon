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
	// Effekseer側でラジアン指定が無いため、度に変換
	float degX = RAD2DEG(rad.x);
	float degY = RAD2DEG(rad.y);
	float degZ = RAD2DEG(rad.z);
	int res = SetRotationPlayingEffekseer3DEffect(handle, degX, degY, degZ);
	if(res == -1)
	{
		return false;
	}
	return true;
}

vec::Vec3 EffekseerManager::GetRotationEffect(int handle) const
{
	if(!_initialize || handle == -1)
	{
		return vec3::VGet(0.0f, 0.0f, 0.0f);
	}

	auto it = _effectRotation.find(handle);
	if(it != _effectRotation.end())
	{
		return vec3::VGet(0.0f, 0.0f, 0.0f);
	}

	return it->second;
}

bool EffekseerManager::SetPosEffect(int handle, const vec::Vec3& pos)
{
	if(handle == -1)
	{
		return false;
	}
	int res = SetPosPlayingEffekseer3DEffect(handle, pos.x, pos.y, pos.z);
	if(res == -1)
	{
		return false;
	}
	return true;
}

vec::Vec3 EffekseerManager::GetPosEffect(int handle) const
{
	if(!_initialize || handle == -1)
	{
		return vec3::VGet(0.0f, 0.0f, 0.0f);
	}
	auto it = _effectPos.find(handle);
	if(it != _effectPos.end())
	{
		return vec3::VGet(0.0f, 0.0f, 0.0f);
	}
	return it->second;
}

bool EffekseerManager::SetSpeedEffect(int handle, float speed)
{
	if(handle == -1)
	{
		return false;
	}
	auto it = _effectSpeed.find(handle);
	if(it != _effectSpeed.end())
	{
		it->second = speed;
	}
	else
	{
		_effectSpeed.emplace(handle, speed);
	}
	return true;
}

float EffekseerManager::GetSpeedEffect(int handle) const
{
	if(!_initialize || handle == -1)
	{
		return 1.0f;
	}
	auto it = _effectSpeed.find(handle);
	if(it != _effectSpeed.end())
	{
		return it->second;
	}
	return 1.0f;
}

bool EffekseerManager::SetScaleEffect(int handle, float scale)
{
	if(handle == -1)
	{
		return false;
	}
	int res = SetScalePlayingEffekseer3DEffect(handle, scale, scale, scale);
	if(res == -1)
	{
		return false;
	}
	return true;
}



// 	// 2Dエフェクト再生
int EffekseerManager::PlayEffect2D(int handle)
{
	if(handle == -1)
	{
		return -1;
	}
	int playing = PlayEffekseer2DEffect(handle);
	if(playing != -1)
	{
		float speed = PlayEffekseer2DEffect(handle);
		_effectSpeed.emplace(playing, speed);
	}
	return playing;
}

// 	// 2Dエフェクト再生(位置指定)
int EffekseerManager::PlayEffect2DPos(int handle, const vec::Vec3& pos)
{
	if(handle == -1)
	{
		return -1;
	}
	int playing = PlayEffekseer2DEffect(handle);
	if(playing != -1)
	{
		SetPosPlayingEffekseer2DEffect(playing, pos.x, pos.y, pos.z);
		float speed = PlayEffekseer2DEffect(handle);
		_effectSpeed.emplace(playing, speed);
	}
	return playing;
}

// 	// 2Dエフェクト停止
bool EffekseerManager::StopEffect2D(int handle)
{
	if(handle == -1)
	{
		return false;
	}
	int res = StopEffekseer2DEffect(handle);
	if(res == -1)
	{
		return false;
	}
	_effectSpeed.erase(handle);
	return true;
}

// 	// 2Dエフェクト再生中かどうか
bool EffekseerManager::IsPlayingEffect2D(int handle) const
{
	if(handle == -1)
	{
		return false;
	}
	int res = IsEffekseer2DEffectPlaying(handle);
	if(res == -1)
	{
		return false;
	}
	return true;
}

// 	// 2Dエフェクト一時停止/再開
bool EffekseerManager::SetPauseEffect2D(int handle, bool pause)
{
	if(handle == -1)
	{
		return false;
	}
	if(pause)
	{
		// 停止
		int res = StopEffekseer2DEffect(handle);
		if(res == -1)
		{
			return false;
		}
	}
	else
	{
		// 再開
		int playing = PlayEffekseer2DEffect(handle);
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

// 	// 2Dエフェクト速度設定
bool EffekseerManager::SetSpeedEffect2D(int handle, float speed)
{
	if(handle == -1)
	{
		return false;
	}
	auto it = _effectSpeed.find(handle);
	if(it != _effectSpeed.end())
	{
		it->second = speed;
	}
	else
	{
		_effectSpeed.emplace(handle, speed);
	}
	return true;
}

float EffekseerManager::GetSpeedEffect2D(int handle) const
{
	if(!_initialize || handle == -1)
	{
		return 1.0f;
	}
	auto it = _effectSpeed.find(handle);
	if(it != _effectSpeed.end())
	{
		return it->second;
	}
	return 1.0f;
}

// 	// 2Dエフェクトスケール設定
bool EffekseerManager::SetScaleEffect2D(int handle, float scale)
{
	if(handle == -1)
	{
		return false;
	}
	int res = SetScalePlayingEffekseer2DEffect(handle, scale, scale, scale);
	if(res == -1)
	{
		return false;
	}
	return true;
}

vec::Vec3 EffekseerManager::GetPosEffect2D(int handle) const
{
	if(!_initialize || handle == -1)
	{
		return vec3::VGet(0.0f, 0.0f, 0.0f);
	}
	auto it = _effectPos.find(handle);
	if(it != _effectPos.end())
	{
		return vec3::VGet(0.0f, 0.0f, 0.0f);
	}
	return it->second;
}

// 	// 2Dエフェクト位置設定
bool EffekseerManager::SetPosEffect2D(int handle, const vec::Vec3& pos)
{
	if(handle == -1)
	{
		return false;
	}
	int res = SetPosPlayingEffekseer2DEffect(handle, pos.x, pos.y, pos.z);
	if(res == -1)
	{
		return false;
	}
	return true;
}

vec::Vec3 EffekseerManager::GetPosEffect2D(int handle) const
{
	if(!_initialize || handle == -1)
	{
		return vec3::VGet(0.0f, 0.0f, 0.0f);
	}
	auto it = _effectPos.find(handle);
	if(it != _effectPos.end())
	{
		return vec3::VGet(0.0f, 0.0f, 0.0f);
	}
	return it->second;
}

// 	// 2Dエフェクト回転設定
bool EffekseerManager::SetRotationEffect2D(int handle, const vec::Vec3& rad)
{
	if(handle == -1)
	{
		return false;
	}
	// Effekseer側でラジアン指定が無いため、度に変換
	float degX = RAD2DEG(rad.x);
	float degY = RAD2DEG(rad.y);
	float degZ = RAD2DEG(rad.z);
	int res = SetRotationPlayingEffekseer2DEffect(handle, degX, degY, degZ);
	if(res == -1)
	{
		return false;
	}
	return true;
}

vec::Vec3 EffekseerManager::GetRotationEffect2D(int handle) const
{
	if(!_initialize || handle == -1)
	{
		return vec3::VGet(0.0f, 0.0f, 0.0f);
	}
	auto it = _effectRotation.find(handle);
	if(it != _effectRotation.end())
	{
		return vec3::VGet(0.0f, 0.0f, 0.0f);
	}
	return it->second;
}

// 	// 2Dエフェクトスケール
bool EffekseerManager::SetScaleEffect2D(int handle, float scale)
{
	if(handle == -1)
	{
		return false;
	}
	int res = SetScalePlayingEffekseer2DEffect(handle, scale, scale, scale);
	if(res == -1)
	{
		return false;
	}
	return true;
}

// 	// 2Dエフェクト更新
void EffekseerManager::Update2D()
{
	if(!_initialize)
	{
		return;
	}
	UpdateEffekseer2D();
}

void EffekseerManager::Render2D()
{
	if(!_initialize)
	{
		return;
	}
	//Effekseer_Sync2DSetting();
	DrawEffekseer2D();
}

void EffekseerManager::Update()
{
	if(!_initialize)
	{
		return;
	}
	UpdateEffekseer3D();
	UpdateEffekseer2D();
}

void EffekseerManager::Render()
{
	if(!_initialize)
	{
		return;
	}

	Effekseer_Sync3DSetting();
	DrawEffekseer3D();
	DrawEffekseer2D();
}