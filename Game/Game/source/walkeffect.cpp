#include "walkeffect.h"
#include "playertanuki.h"
#include "enemysoundmanager.h"

WalkEffect::WalkEffect()
{
	_playerBase = nullptr;
	_stepIntervalFrames = 12; // 約0.2秒(60fps想定)。必要なら調整してください。
	_stepCounter = 0;
	_wasDash = false;
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
	StopPlaying();

	auto em = EffekseerManager::GetInstance();
	if(em && _handle != -1)
	{
		em->DeleteEffect(_handle);
		_handle = -1;
	}

	return true;
}

bool WalkEffect::StopPlaying()
{
	bool isStop = false;
	auto em = EffekseerManager::GetInstance();

	// このインスタンスが記録している全ての再生ハンドルを停止
	for(auto h : _playHandles)
	{
		if(h != -1 && em)
		{
			em->StopEffect(h);
			isStop = true;
		}
	}
	_playHandles.clear();

	// EffectBase が持つ単一ハンドルも停止（念のため）
	if(base::StopPlaying())
	{
		isStop = true;
	}
	_playHandle = -1;

	return isStop;
}

bool WalkEffect::Process()
{
	base::Process();

	// プレイヤー未設定なら何もしない
	if(!_playerBase) return true;

	PlayerTanuki* tanuki = dynamic_cast<PlayerTanuki*>(_playerBase);
	bool isDash;
	if(tanuki != nullptr)
	{
		isDash = tanuki->IsDash();
	}
	else
	{
		isDash = false; // プレイヤーが PlayerTanuki でない場合はダッシュしていないとみなす
	}

	if(isDash && !_wasDash)
	{
		vec::Vec3 pos = _playerBase->GetPos();
		auto em = EffekseerManager::GetInstance();
		if(em && _handle != -1)
		{
			int playHandle = em->PlayEffect3DPos(_handle, pos);
			if(playHandle != -1)
			{

				_playHandles.emplace_back(playHandle);
			}
		}

		EnemySoundManager::GetInstance()->EmitSound(
			_playerBase->GetPos(),  // 宝箱の位置
			5,						// 音の大きさレベル（1-3で調整）
			400.0f,					// 音波の最大半径
			10.0f					// 音波の速度
		);
		_stepCounter = _stepIntervalFrames; // ダッシュ開始時にカウンタリセットして即発生させる
	}

	_wasDash = isDash; // 今のダッシュ状態を記録して次フレームで比較するために保存

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
				// ResetStage 等で StopPlaying() が有効に働くようにする
				int playHandle = em->PlayEffect3DPos(_handle, pos);
				if(playHandle != -1)
				{
					//SetSpeedPlayingEffekseer3DEffect(playHandle, 10.0f); // 速度を0にして固定
					_playHandles.emplace_back(playHandle);
				}
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