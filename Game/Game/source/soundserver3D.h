#pragma once
#include "appframe.h"
#include "applicationglobal.h"

namespace soundserver
{
	class SoundServer;
	class SoundItemBase;
}

class CharaBase;

class SoundServer3D
{
public:
	struct VoiceInstance
	{
		int handle;
		soundserver::SoundItemBase* sound;
		float radius= 768.0f;
	};


	SoundServer3D(at::spc<soundserver::SoundServer> soundServer);
	~SoundServer3D() = default;

	// 3DâπåπÇÃîºåaê›íË
	void SetRadius(float rad);

	// 3DâπåπÇÃà íuçXêV
	void PlayLoopSound3D(void* key, const std::string& soundId, const vec::Vec3& pos);

	void StopSound3D(void* key);

	void StopAll();

private:
	void Apply3D(VoiceInstance& instance, const vec::Vec3& pos) const;

	at::spc<soundserver::SoundServer> _soundServer;
	float _radius;

	at::umtt<void*, VoiceInstance> _instance;
};

