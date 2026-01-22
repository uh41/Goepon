#include "animationmanager.h"

AnimationManager* AnimationManager::GetInstance()
{
	static AnimationManager instance;
	return &instance;
}

AnimationManager::AnimationManager()
{
	_nextId = 1;
}

AnimationManager::~AnimationManager()
{
	Terminate();
}

bool AnimationManager::Terminate()
{
	for(auto& anim : _animInstance)
	{
		DetachInstance(anim.first);
	}
	_animInstance.clear();
	return true;
}

int AnimationManager::Play(int handle, std::string& name, bool loop, float speed)
{
	if(handle == -1)
	{
		return -1;
	}

	// アニメーションインデックス取得
	int animIdx = MV1GetAnimIndex(handle, name.c_str());
	if(animIdx == -1)
	{
		return -1;
	}

	// アタッチインデックス取得
	int attachIndex = MV1AttachAnim(handle, animIdx);
	if(attachIndex == -1)
	{
		return -1;
	}

	float total = static_cast<float>(MV1GetAttachAnimTotalTime(handle, attachIndex));
	if(total <= 0.0f)
	{
		return -1;
	}

	return CreateInstance(handle, attachIndex, name, total, loop, speed);

}

void AnimationManager::Stop(int id)
{
	auto it = _animInstance.find(id);
	if(it != _animInstance.end())
	{
		DetachInstance(id);
		_animInstance.erase(it);
	}
}

void AnimationManager::StopAllModel(int handle)
{
	for(auto it = _animInstance.begin(); it != _animInstance.end(); )
	{
		if(it->second.handle == handle)
		{
			DetachInstance(it->first);
			it = _animInstance.begin();
		}
		else
		{
			++it;
		}
	}
}

// アニメーションインスタンスのデタッチ
void AnimationManager::DetachInstance(int id)
{
	auto it = _animInstance.find(id);
	if(it == _animInstance.end())
	{
		return;
	}

	Instance& instance = it->second;
	// MV1のデタッチ
	if(instance.attachIndex != -1 && instance.handle != -1)
	{
		MV1DetachAnim(instance.handle, instance.attachIndex);
	}
}

void AnimationManager::Update(float time)
{
	if(_animInstance.empty())
	{
		return;
	}

	for(auto it = _animInstance.begin(); it != _animInstance.end();)
	{
		Instance& instance = it->second;
		// 再生中でなければスキップ
		if(!instance.playing)
		{
			++it;
			continue;
		}

		instance.playTime += time * instance.speed;

		// 再生時間が総再生時間を超えたら
		if(instance.totalTime > 0.0f && instance.playTime >= instance.totalTime)
		{
			// ループ再生なら再生時間を調整
			if(instance.loop)
			{
				instance.playTime = std::fmod(instance.playTime, instance.totalTime); // ループ再生(std::fmodで余りを取得)
				if(instance.playTime < 0.0f)
				{
					instance.playing = 0.0f;
				}
			}
			else
			{
				// 再生終了
				MV1SetAttachAnimTime(instance.handle, instance.attachIndex, instance.totalTime);
				MV1DetachAnim(instance.handle, instance.attachIndex);
				it = _animInstance.erase(it);
				continue;
			}
		}

		// 再生時間設定
		MV1SetAttachAnimTime(instance.handle, instance.attachIndex, instance.playTime);
		++it;
	}
}

bool AnimationManager::SetTime(int id, float time)
{
	auto it = _animInstance.find(id);
	if(it == _animInstance.end())
	{
		return false;
	}

	Instance& instance = it->second;
	instance.playTime = time;

	// 再生時間設定
	if(instance.playTime < 0.0f)
	{
		instance.playTime = 0.0f;	// 負の値は0に補正
	}

	// 再生時間が総再生時間を超えたら
	if(instance.totalTime > 0.0f && instance.playTime > instance.totalTime)
	{
		// ループ再生なら再生時間を調整
		if(instance.loop)
		{
			instance.playTime = std::fmod(instance.playTime, instance.totalTime); // ループ再生(std::fmodで余りを取得)
			if(instance.playTime < 0.0f)
			{
				instance.playTime += instance.totalTime;
			}
		}
		else
		{
			instance.playTime = instance.totalTime; // 総再生時間を超えたら総再生時間に補正
		}
	}

	MV1SetAttachAnimTime(instance.handle, instance.attachIndex, instance.playTime);

	return true;
}

// 一時停止/再開
bool AnimationManager::SetPlaying(int id, bool play)
{
	auto it = _animInstance.find(id);
	if(it == _animInstance.end())
	{
		return false;
	}

	it->second.playing = play;// 再生状態設定
	return true;
}

bool AnimationManager::IsPlaying(int id) const
{
	auto it = _animInstance.find(id);
	if(it == _animInstance.end())
	{
		return false;
	}
	return it->second.playing;
}

int AnimationManager::CreateInstance(int handle, int attachindex, const std::string& name,float totaltime, bool loop, float speed)
{
	Instance instance;
	instance.id = _nextId++;
	instance.handle = handle;
	instance.attachIndex = attachindex;
	instance.name = name;
	instance.totalTime = totaltime;
	instance.playTime = 0.0f;
	instance.speed = speed;
	instance.loop = loop;
	instance.playing = true;
	_animInstance[instance.id] = instance;
	return instance.id;
}