/*********************************************************************/
// * \file   charabase.h
// * \brief  キャラベースクラス
// *
// * \author 鈴木裕稀
// * \date   2025/12/15
// * \作業内容: 新規作成 鈴木裕稀　2025/12/15
/*********************************************************************/

#pragma once
#include "objectbase.h"
#include "camera.h"

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
		_EOT_
	};
	STATUS _status;

	virtual bool Initialize();
	virtual bool Terminate();
	virtual bool Process();
	virtual bool Render();

	const VECTOR& GetPos() const { return _vPos; }
	const VECTOR& GetOldPos() const { return _vOldPos; }
	auto GetColSubY() const { return _fColSubY; }

	const VECTOR& SetPos(const VECTOR& pos) { return _vPos = pos; }
	const VECTOR& GetDir() const { return _vDir; }
	void SetDir(const VECTOR& dir) { _vDir = dir; }

	auto GetCollisionR() const { return _fCollisionR; }
	auto GetCollisionWeight() const { return _fCollisionWeight; }

	bool Damage(float damage);

	float GetHP() const { return _fHp; }	// HP取得用ゲッター
	bool IsAlive() const { return _bIsAlive; } // 生存確認用ゲッター

protected:
	float _iAttachIndex;
	float _fTotalTime;
	float _fPlayTime;
	VECTOR _vPos; // キャラの位置
	VECTOR _vOldPos; // 1フレーム前のキャラの位置
	VECTOR _vDir; // キャラの向き
	float _fColSubY; // コリジョン判定用のYオフセット
	float _fCollisionR; // 円の半径
	float _fCollisionWeight; // キャラの重さ

	float _fHp;// キャラの体力
	bool _bIsAlive;// 生存フラグ
};

