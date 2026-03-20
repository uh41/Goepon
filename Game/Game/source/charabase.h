/*********************************************************************/
// * \file   charabase.h
// * \brief  キャラベースクラス
// *
// * \author 鈴木裕稀
// * \date   2025/12/15
// * \作業内容: 新規作成 鈴木裕稀　2025/12/15
//				vec::Vec3を使用するように修正　鈴木裕稀　2026/01/17
/*********************************************************************/

#pragma once
#include "objectbase.h"
#include "camera.h"

namespace rad
{
	static constexpr auto ROTATION_SPEED = 20.0f; // 90度をラジアンで表した値
}

class CharaBase : public ObjectBase
{
	typedef ObjectBase base;
public:
	enum class STATUS
	{
		NONE,
		WAIT,
		WALK,
		JUMP,
		FALL,
		ATTACK,
		LANDING,
		DASHING,
		ROLLING,
		FOUND,
		_EOT_
	};
	STATUS _status;

	virtual bool Initialize();
	virtual bool Terminate();
	virtual bool Process();
	virtual bool Render();

	// 位置は ObjectBase::_vPos を使う（重複メンバは持たない）
	const vec::Vec3& GetPos() const
	{
		return _vPos;
	}
	const vec::Vec3& GetOldPos() const
	{
		return _vOldPos;
	}
	void SetOldPos(const vec::Vec3& pos)
	{
		_vOldPos = pos;
	}
	auto GetColSubY() const {
		return _fColSubY;
	}

	// 明示的にベースの SetPos を呼ぶ実装（呼び側は戻り値を使わない想定）
	void SetPos(const vec::Vec3& pos)
	{
		base::SetPos(pos);
	}

	const vec::Vec3& GetDir() const
	{
		return _vDir;
	}

	auto GetCollisionR() const
	{
		return _fCollisionR;
	}
	auto GetCollisionWeight() const
	{
		return _fCollisionWeight;
	}

	bool Damage(float damage);

	float GetHP() const
	{
		return _fHp;
	}	// HP取得用ゲッター
	bool IsAlive() const 
	{
		return _bIsAlive;
	} // 生存確認用

	// 生存フラグ
	void SetAlive(bool alive) { _bIsAlive = alive; }

	// アニメーション関連
	int PlayAnimation(std::string name, bool loop = false); // アニメーション再生。
	void StopAnimation();									// アニメーション停止。
	int GetAnimId() const { return _animId; }				// 現在のアニメーションIDを取得
	bool IsAnimationPlaying() const;						// アニメーションが再生中かどうかを確認
	void ClearAnimIdIfStopped();							// アニメーションが停止している場合、アニメーションIDをクリアする
	float GetAnimPlayTime() const { return _fPlayTime; }    // 現在のアニメーションの再生時間を取得するゲッター
	float GetAnimTotalTime() const { return _fTotalTime; }  // 現在のアニメーションの総時間を取得するゲッター

	const vec::Vec3& GetInputVector() const
	{
		return _vInput;
	}
	float GetMoveSpeed() const
	{
		return _fMvSpeed;
	}

	auto GetLand() const
	{
		return _bLand;
	}
	void SetLand(bool land)
	{
		_bLand = land;

		_vDir.x = sinf(_rotationY);
		_vDir.z = cosf(_rotationY);
		_vDir = vec3::VNorm(_vDir);
	}

	void UpdateRotation();
	void SetTargetRotationFromDirection(const vec::Vec3& dir);
	auto GetRotationY() const { return _rotationY; }
	void SetRotationY(float rotationY) { _rotationY = rotationY; }
	void SetTargetRotationY(float rotation) { _targetRotationY = rotation; }

protected:
	float _iAttachIndex;
	float _fTotalTime;
	float _fPlayTime;
	// _vPos は ObjectBase に定義済みのためここでは定義しない
	vec::Vec3 _vOldPos; // 1フレーム前のキャラの位置

	float _fColSubY; // コリジョン判定用のYオフセット
	float _fCollisionR; // 円の半径
	float _fCollisionWeight; // キャラの重さ

	float _fHp;// キャラの体力
	bool _bIsAlive;// 生存フラグ

	float   _fMvSpeed;
	vec::Vec3 _vInput;

	bool    _bLand; // 着地フラグ

	float _rotationY; // プレイヤーのY軸回転角
	float _targetRotationY; // プレイヤーのY軸回転の目標値
};