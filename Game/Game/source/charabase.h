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

	const VECTOR& GetPos() const { return _pos; }
	const VECTOR& GetOldPos() const { return _old_pos; }
	auto GetColSubY() const { return _col_sub_y; }

	const VECTOR& SetPos(const VECTOR& pos) { return _pos = pos; }
	auto GetCollisionR() const { return _collision_r; }
	auto GetCollisionWeight() const { return _collision_weight; }

	bool Damage(float damage);

	float GetHP() const { return _hp; }	// HP取得用ゲッター
	bool IsAlive() const { return _is_alive; } // 生存確認用ゲッター

protected:
	int _attach_index;
	float _total_time;
	float _play_time;
	VECTOR _pos; // キャラの位置
	VECTOR _old_pos; // 1フレーム前のキャラの位置
	VECTOR _dir; // キャラの向き
	float _col_sub_y; // コリジョン判定用のYオフセット
	float _collision_r; // 円の半径
	float _collision_weight; // キャラの重さ

	float _hp;// キャラの体力
	bool _is_alive;// 生存フラグ
};

