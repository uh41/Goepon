#include "charabase.h"

// 初期化
bool CharaBase::Initialize()
{
	if(!base::Initialize()) { return false; }
	_handle = -1;
	_attach_index = -1;
	_total_time = 0.0f;
	_play_time = 0.0f;
	_pos = VGet(0.0f, 0.0f, 0.0f);
	_old_pos = VGet(0.0f, 0.0f, 0.0f);
	_dir = VGet(0.0f, 0.0f, 0.0f);
	_col_sub_y = 0.0f;
	_collision_r = 0.0f;
	_collision_weight = 0.0f;
	_status = STATUS::NONE;
	_is_alive = true; // 生存フラグを初期化
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
	_hp -= damage;

	// 0以下になっていないか確認
	if(_hp <= 0.0f)
	{
		_hp = 0.0f;
		_is_alive = false; // HPが0になったら生存フラグをfalseにする
	}

	return true;
}

// 描画処理
bool CharaBase::Render()
{
	base::Render();
	
	return true;
}


