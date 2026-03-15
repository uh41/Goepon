/*********************************************************************/
// * \file   charabase.cpp
// * \brief  キャラベースクラス
// *
// * \author 鈴木裕稀
// * \date   2025/12/15
// * \作業内容: 新規作成 鈴木裕稀　2025/12/15
//				vec::Vec3を使用するように修正　鈴木裕稀　2026/01/17
/*********************************************************************/

#include "charabase.h"
#include "appframe.h"

// 初期化
bool CharaBase::Initialize()
{
	if(!base::Initialize()) { return false; }
	_handle = -1;
	_iAttachIndex = -1;
	_fTotalTime = 0.0f;
	_fPlayTime = 0.0f;
	_vPos = vec3::VGet(0.0f, 0.0f, 0.0f);
	_vOldPos = vec3::VGet(0.0f, 0.0f, 0.0f);
	_vDir = vec3:: VGet(0.0f, 0.0f, 0.0f);
	_rotationY = 0.0f; // Y軸回転角を初期化
	_fColSubY = 0.0f;
	_fCollisionR = 0.0f;
	_fCollisionWeight = 0.0f;
	_rotationY = 0.0f;
	_targetRotationY = 0.0f;
	_status = STATUS::NONE;
	_bIsAlive = true; // 生存フラグを初期化

	TextUtil::GetInstance()->GetConfig(_config, "HP", _fHp);// 初期体力設定
	TextUtil::GetInstance()->GetConfig(_config, "speed", _fMvSpeed);// 移動速度設定

	return true;
}

// 終了
bool CharaBase::Terminate()
{
	base::Terminate();
	return true;
}

int CharaBase::PlayAnimation(std::string name, bool loop)
{
	if(_animId != -1)
	{
		AnimationManager::GetInstance()->Stop(_animId);
		_animId = -1;
	}

	if(_handle == -1 || name.empty())
	{
		return -1;
	}

	_animId = AnimationManager::GetInstance()->Play(_handle, name, loop);
	if(_animId != -1)
	{
		AnimationManager::GetInstance()->SetTime(_animId, 0.0f);
	}
	return _animId;
}



void CharaBase::StopAnimation()
{
	if(_animId != -1)
	{
		AnimationManager::GetInstance()->Stop(_animId);
		_animId = -1;
	}
}

// 計算処理
bool CharaBase::Process()
{
	base::Process();
	return true;
}

bool CharaBase::Damage(float damage)
{
	// ダメージ量を減らす
	_fHp -= damage;

	// 0以下になっていないか確認
	if(_fHp <= 0.0f)
	{
		_fHp = 0.0f;
		_bIsAlive = false; // HPが0になったら生存フラグをfalseにする
	}

	return true;
}

void CharaBase::UpdateRotation()
{
	// 目標角度と現在角度の差分を計算
	float angleDiff = _targetRotationY - _rotationY;

	// 角度差を -π～π の範囲に正規化
	while(angleDiff > DX_PI_F)
	{
		angleDiff -= DX_TWO_PI_F;
	}
	while(angleDiff < -DX_PI_F)
	{
		angleDiff += DX_TWO_PI_F;
	}

	// 20度をラジアンに変換（度→ラジアン）
	float rotationStep = DEG2RAD(rad::ROTATION_SPEED);

	if(fabsf(angleDiff) > rotationStep)
	{
		// 差分が大きい場合は20度ずつ回転
		if(angleDiff > 0.0f)
		{
			_rotationY += rotationStep;
		}
		else
		{
			_rotationY -= rotationStep;
		}
	}
	else
	{
		// 差分が小さい場合は目標角度にぴったり合わせる
		_rotationY = _targetRotationY;
	}

	// 角度を -π～π の範囲に正規化
	while(_rotationY > DX_PI_F)
	{
		_rotationY -= DX_TWO_PI_F;
	}
	while(_rotationY < -DX_PI_F)
	{
		_rotationY += DX_TWO_PI_F;
	}
}

void CharaBase::SetTargetRotationFromDirection(const vec::Vec3& dir)
{
	if(vec3::VSize(dir) > 0.0f) // 入力ベクトルがほぼゼロでない場合のみ回転を更新
	{
		_targetRotationY = atan2f(dir.x * -1.0f, dir.z * -1.0f); // XZ平面の角度を計算
	}
}

// 描画処理
bool CharaBase::Render()
{
	base::Render();
	
	return true;
}


