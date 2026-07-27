/*********************************************************************/
// * \file   soundserver3D.h
// * \brief  3D音源管理クラス
// *
// * \author 鈴木裕稀
/*********************************************************************/

#pragma once
#include "appframe.h"
#include "applicationglobal.h"

namespace soundserver
{
	class SoundServer;
	class SoundItemBase;
}

class CharaBase;

namespace sound
{
	static constexpr float DEFAULT_RADIUS = 768.0f;	// デフォルトの3D音源の半径
}

class SoundServer3D
{
public:
	struct VoiceInstance
	{
		int handle;
		soundserver::SoundItemBase* sound;
		float radius= sound::DEFAULT_RADIUS;
	};


	SoundServer3D(at::spc<soundserver::SoundServer> soundServer);
	~SoundServer3D() = default;

	// 3D音源の半径設定
	void SetRadius(float rad);

	// 3D音源の位置更新
	void PlayLoopSound3D(void* key, const std::string& soundId, const vec::Vec3& pos);

	// 3D音源を停止
	void StopSound3D(void* key);

	// 3D音源を全て停止
	void StopAll();

private:
	
	void Apply3D(VoiceInstance& instance, const vec::Vec3& pos) const;// 3D音源の適用

	at::spc<soundserver::SoundServer> _soundServer;
	float _radius;

	at::umtt<void*, VoiceInstance> _instance;// 3D音源のインスタンス管理
};

