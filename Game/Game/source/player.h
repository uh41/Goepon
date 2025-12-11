#pragma once

#include "playerbase.h"
#include "camera.h"
class Player : public PlayerBase
{
	typedef PlayerBase base;
public:
	virtual bool Initialize();
	virtual bool Terminate();
	virtual bool Process();
	virtual bool Render();


	const VECTOR& GetInputVector() const { return _input_v; }
	float GetMoveSpeed() const { return _mv_speed; }
	auto GetLand() const { return _land; }
	void SetLand(bool land) { _land = land; }

	void SetCamera(Camera* cam)  override { _cam = cam; }

	// 攻撃用カプセル当たり判定
	bool AttackCapsule
	(
		VECTOR underpos,	// カプセルの下位置
		VECTOR overpos,		// カプセルの上下位置
		float r,			// 半径
		int waittime,		// 発生までの時間
		int activetime,		// 有効時間
		int timespeed,		// カプセルの伸縮速度
		bool follow,		// カプセルがキャラに追従するか		
		float damage,		// ダメージ量
		int framenum,		// ノックバックフレーム数
		VECTOR dir
	);

	bool PlayerMove(VECTOR v);

	bool Attack();

	std::vector<mymath::ATTACKCOLLISION>& GetAttackCollisionList() { return _attack_collision; }

protected:
	Camera* _cam;
	float _mv_speed;
	VECTOR _input_v;
	
	bool _land;// 着地しているかどうか 
	float _jump_height; // ジャンプの高さ
	float _gravity; // 重力の強さ

	bool _is_dashing;// ダッシュ中かどうか
	float _dash_speed;
	float _dash_time;
	float _dash_timer;
	VECTOR _dash_direction;

	
	bool _is_rolling; 
	float _roll_speed; 
	float _roll_time; 
	float _roll_timer;			// ドッジロール残り時間
	VECTOR _roll_direction;		// ドッジロール方向

	bool _jump_count;			// ジャンプ回数制限用フラグ
	float _air_control;			// 空中制御係数

	bool _is_attack;			// 攻撃中かどうか
	bool _air_attack_used;		// 空中攻撃を使用したかどうか

	int _axis_hold_count;		// 十字キー水平入力保持カウント
	bool _axis_lock;			// 十字キー水平入力ロックフラグ
	int _hold_threshold;		// 十字キー水平入力保持閾値

	VECTOR _axis_lock_dir;

	std::vector<mymath::ATTACKCOLLISION> _attack_collision; // 攻撃用カプセル当たり判定リスト

};

