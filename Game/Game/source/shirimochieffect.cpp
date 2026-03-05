#include "shirimochieffect.h"

ShirimochiEffect::ShirimochiEffect()
{

}

bool ShirimochiEffect::Initialize()
{
	base::Initialize();
	_handle = EffekseerManager::GetInstance()->LoadEffect(ef::EF_Shirimoti);
	return true;
}

bool ShirimochiEffect::Terminate()
{
	base::Terminate();
	auto em = EffekseerManager::GetInstance();
	if(em && _handle != -1)
	{
		em->DeleteEffect(_handle);
		_handle = -1;
	}
	return true;
}

void ShirimochiEffect::PlayEffect(const vec::Vec3& pos)
{
    auto em = EffekseerManager::GetInstance();
    if(!em)
    {
        return;
    }

    if(_handle == -1)
    {
        return; // エフェクトファイルが読み込まれていない
    }

    // 既に再生中なら停止
    if(_playHandle != -1)
    {
        em->StopEffect(_playHandle);
        _playHandle = -1;
    }

    // エフェクト再生
    _playHandle = em->PlayEffect3DPos(_handle, pos);
}

bool ShirimochiEffect::Process()
{
	base::Process();
	return true;
}

bool ShirimochiEffect::Render()
{
	base::Render();
	return true;
}

