#include "TitleTanuki.h"

bool TitleTanuki::Initialize()
{
	base::Initialize();
	LoadModel(mv1::SK_goepon_multimotion_4);
	_iAttachIndex = -1;
	_status = STATUS::WAIT;

	_vPos = vec::Vec3(0.0f, 0.0f, 0.0f);
	_vDir = vec::Vec3(0.0f, 0.0f, 0.0f);
	// アニメーション関係
	_fTotalTime = 0.0f;
	_fPlayTime = 0.0f;

	return true;
}

bool TitleTanuki::Terminate()
{
	return true;
}

bool TitleTanuki::Process()
{
	base::Process();


	// タイトルは常に待機（WAIT）扱い
	_status = STATUS::WAIT;

	// プレイヤーと同様：再生が切れていたら再生し直す
	if(_animId != -1 && !AnimationManager::GetInstance()->IsPlaying(_animId))
	{
		_animId = -1;

		std::string animName = "idle";
		_animId = AnimationManager::GetInstance()->Play(_handle, animName, true);
		if(_animId != -1)
		{
			_fTotalTime = AnimationManager::GetInstance()->GetTotalTime(_animId);
			_fPlayTime = 0.0f;
			AnimationManager::GetInstance()->SetTime(_animId, _fPlayTime);
		}
	}

	// 再生時間の管理（PlayerTanuki に合わせた進め方）
	{
		float animSpeed = 0.5f;
		_fPlayTime += animSpeed;

		// WAIT時の微妙な揺らぎ（プレイヤー側のロジックを踏襲）
		_fPlayTime += (float)(rand() % 10) / 100.0f;

		if(_animId != -1)
		{
			AnimationManager::GetInstance()->SetTime(_animId, _fPlayTime);
		}
	}

	// 念のためループ制御（AnimationManager 側もループするが、_fPlayTime を暴走させない）
	if(_fTotalTime > 0.0f && _fPlayTime >= _fTotalTime)
	{
		_fPlayTime = 0.0f;
	}

	return true;
}

bool TitleTanuki::Render()
{
	base::Render();

	float vorty = atan2(_vDir.x * -1, _vDir.z * -1);

	float ScaleOffset = 1.5f;
	MATRIX mRotY = MGetRotY(vorty);
	MATRIX mRotZ = MGetRotZ(DX_PI_F * 0.5f);
	MATRIX mTrans = MGetTranslate(DxlibConverter::VecToDxLib(_vPos));
	MATRIX mScale = MGetScale(VGet(ScaleOffset, ScaleOffset, ScaleOffset));

	MATRIX m = MGetIdent();
	//m = MMult(m, mRotZ);
	//m = MMult(m, mRotY);
	m = MMult(m, mScale);
	m = MMult(m, mTrans);

	MV1SetMatrix(_handle, m);
	MV1DrawModel(_handle);
	return true;
}