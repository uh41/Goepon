#include "AttackUi.h"
#include "modegame.h"
#include "playertanuki.h"

AttackUi::AttackUi()
{
	_handle = -1;

	_pos = vec::Vec3(0.0f, 0.0f, 0.0f);
	_visible = false;
	_size = 100.0f;
	_angle = 0.0f;
}

bool AttackUi::Initialize()
{
	base::Initialize();
	_handle = LoadGraph(ui::TailAtacck);
	return true;
}

bool AttackUi::Terminate()
{
	base::Terminate();
	if(_handle != -1)
	{
		DeleteGraph(_handle);
		_handle = -1;
	}
	return true;
}

bool AttackUi::Process()
{
	base::Process();
	return true;
}

bool AttackUi::Render()
{
	base::Render();

	if(!_visible || _handle == -1)
	{
		return false;
	}

	auto modeGame = dynamic_cast<ModeGame*>(ModeServer::GetInstance()->Get("game"));
	if(!modeGame)
	{
		return false;
	}

	// タヌキやモノの表示中は攻撃UIを表示しない
	if(modeGame->GetShowTanuki() || modeGame->IsShowingMono())
	{
		return false;
	}

	// プレイヤーの情報を取得
	auto player = modeGame->GetPlayer();
	if(!player)
	{
		return false;
	}

	// 攻撃範囲内の敵を取得
	auto& enemy = modeGame->GetEnemiesInAttackRangees();

	// プレイヤーの座標を取得し、Y座標を上にオフセット
	if(!enemy.empty())
	{
		vec::Vec3 playerPos = player->GetPos();
		playerPos.z += attack::PLAYER_HEAD_Y;
		VECTOR pos = DxlibConverter::VecToDxLib(playerPos); 


		// 3D空間上に描画
		DrawBillboard3D(pos, 0.5f, 0.5f, _size, _angle, _handle, TRUE);
	}
	return true;
}