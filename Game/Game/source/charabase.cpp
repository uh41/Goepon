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
	_fColSubY = 0.0f;
	_fCollisionR = 0.0f;
	_fCollisionWeight = 0.0f;
	_status = STATUS::NONE;
	_bIsAlive = true; // 生存フラグを初期化
	return true;
}

// 終了
bool CharaBase::Terminate()
{
	base::Terminate();
	return true;
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

// 描画処理
bool CharaBase::Render()
{
	base::Render();
	
	return true;
}


