#include "tutorial.h"

Tutorial::Tutorial()
{
	_eventId = 0;
	Initialize();
}

bool Tutorial::Initialize()
{
	base::Initialize();
	LoadModel(mv1::S_Marker_Event);
	// 当たり判定用フレームの取得
	_tutorialCollisionFrame = MV1SearchFrame(_handle, "Collision");
	_attachIndex = -1;
	_vPos = vec::Vec3{ 0.0f, 0.0f, 0.0f };
	_vDir = vec::Vec3{ 0.0f, 0.0f, -1.0f };
	MV1SetupCollInfo(_handle, _tutorialCollisionFrame, 16, 16, 16);
	MV1SetFrameVisible(_handle, _tutorialCollisionFrame, FALSE);
	_isVisible = true;
	// 当たり判定情報の初期更新
	ApplyMatrixAndRefreshCollInfo(_handle, _tutorialCollisionFrame, -1, MakeModelMatrix());

	LoadEffect();
	return true;
}

bool Tutorial::Terminate()
{
	_handle = -1;
	_tutorialCollisionFrame = -1;
	_isVisible = false;
	_eventHandle.clear();
	return true;
}

void Tutorial::LoadEffect()
{
	// イベントIDとエフェクトハンドルの対応を設定
	_eventHandle["event1"] = EffekseerManager::GetInstance()->LoadEffect(ef::tutorial_1);
	_eventHandle["event2"] = EffekseerManager::GetInstance()->LoadEffect(ef::tutorial_2);
	_eventHandle["event3"] = EffekseerManager::GetInstance()->LoadEffect(ef::tutorial_3);
	_eventHandle["event4"] = EffekseerManager::GetInstance()->LoadEffect(ef::tutorial_4);
	_eventHandle["event5"] = EffekseerManager::GetInstance()->LoadEffect(ef::tutorial_5);
	_eventHandle["event6"] = EffekseerManager::GetInstance()->LoadEffect(ef::tutorial_6);
	_eventHandle["event7"] = EffekseerManager::GetInstance()->LoadEffect(ef::tutorial_7);
	_eventHandle["event8"] = EffekseerManager::GetInstance()->LoadEffect(ef::tutorial_8);
	_eventHandle["event9"] = EffekseerManager::GetInstance()->LoadEffect(ef::tutorial_9);
}

void Tutorial::PlayTutorial()
{
	std::string effectName;
	
	switch(_eventId)
	{
	case 1:
	{
		effectName = "event1";
		break;
	}
	case 2:
	{
		effectName = "event2";
		break;
	}
	case 3:
	{
		effectName = "event3";
		break;
	}
	case 4:
	{
		effectName = "event4";
		break;
	}
	case 5:
	{
		effectName = "event5";
		break;
	}
	case 6:
	{
		effectName = "event6";
		break;
	}
	case 7:
	{
		effectName = "event7";
		break;
	}
	case 8:
	{
		effectName = "event8";
		break;
	}
	case 9:
	{
		effectName = "event9";
		break;
	}
	}

	auto it = _eventHandle.find(effectName);
	if(it != _eventHandle.end())
	{
		int effectHandle = it->second;
		EffekseerManager::GetInstance()->PlayEffect3DPos(effectHandle, _vPos);
	}
}

bool Tutorial::Process()
{
	base::Process();
	// 当たり判定情報の更新
	ApplyMatrixAndRefreshCollInfo(_handle, _tutorialCollisionFrame, -1, MakeModelMatrix());
	return true;
}

bool Tutorial::Render()
{
	base::Render();
	// 非表示なら描画しない（デバック)
	if(!_isVisible)
	{
		return false;
	}

	if(_handle != -1)
	{
		MV1DrawModel(_handle);
	}

	return true;
}

MATRIX Tutorial::MakeModelMatrix() const
{
	float vorty = atan2(_vDir.x * -1, _vDir.z * -1);
	MATRIX mRotY = MGetRotY(vorty);
	MATRIX mTrans = MGetTranslate(DxlibConverter::VecToDxLib(_vPos));
	MATRIX mScale = MGetScale(VGet(1.2f, 1.2f, 1.2f));
	MATRIX m = MGetIdent();
	m = MMult(m, mScale);
	m = MMult(m, mRotY);
	m = MMult(m, mTrans);
	return m;
}