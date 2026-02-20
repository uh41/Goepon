#include "walkeffect.h"

WalkEffect::WalkEffect()
{
	_playerBase = nullptr;
	_stepIntervalFrames = 12; // 約0.2秒(60fps想定)。必要なら調整してください。
	_stepCounter = 0;
	Initialize();
}

bool WalkEffect::Initialize()
{
	base::Initialize();
	// ef::EF_walk をロード
	_handle = EffekseerManager::GetInstance()->LoadEffect(ef::EF_walk, 1.0f);
	return true;
}

bool WalkEffect::Terminate()
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

bool WalkEffect::Process()
{
	base::Process();

	// プレイヤー未設定なら何もしない
	if(!_playerBase) return true;

	// プレイヤーが歩行中なら一定間隔でエフェクトを発生（発生位置はその瞬間の座標を渡し固定する）
	if(_playerBase->_status == CharaBase::STATUS::WALK)
	{
		if(_stepCounter <= 0)
		{
			// 発生位置を取得（プレイヤー位置そのもの。必要なら Y を調整してください）
			vec::Vec3 pos = _playerBase->GetPos();

			// Effekseer に発生時の座標で再生（再生後は位置更新しない -> 固定）
			auto em = EffekseerManager::GetInstance();
			if(em && _handle != -1)
			{
				em->PlayEffect3DPos(_handle, pos);
			}

			// カウンタリセット
			_stepCounter = _stepIntervalFrames;
		}
		else
		{
			--_stepCounter;
		}
	}
	else
	{
		// 歩行でないときはカウンタリセットしておく（次に歩き始めた瞬間に即発生させたい場合はここを調整）
		_stepCounter = 0;
	}

	return true;
}

bool WalkEffect::Render()
{
	base::Render();
	return true;
}