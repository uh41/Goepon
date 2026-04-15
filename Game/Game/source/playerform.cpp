#include "playerform.h"
#include "PlayerFactory.h"
#include "effectmanager.h"
#include "applicationglobal.h"
#include "playertanuki.h"

PlayerForm* PlayerForm::GetInstance()
{
	static PlayerForm instance;
	return &instance;
}

bool PlayerForm::Initialize()
{
	_playerType = PlayerBase::PlayerType::TANUKI;
	return true;
}

bool PlayerForm::Terminate()
{
	return true;
}

PlayerBase* PlayerForm::GetPlayer() const
{
	return PlayerFactory::GetPlayer(_playerType);
}

void PlayerForm::ChangeState(PlayerBase::PlayerType type)
{
	if(_playerType == type)
	{
		return;
	}

	PlayerBase* from = PlayerFactory::GetPlayer(_playerType);
	PlayerBase* to = PlayerFactory::GetPlayer(type);
	if(!to)
	{
		return;
	}

	// ó‘Ô‚ðˆø‚«Œp‚¬
	if(from)
	{
		to->CopyStateFrom(from);
	}

	_playerType = type;

	EffectManager::UpdatePalyerTransformEffect(to, true);
	EffectManager::UpdatePlayerPosition(to);

	auto soundHenshin = gGlobal._soundServer->Get("2");
	if(soundHenshin && !soundHenshin->IsPlay())
	{
		soundHenshin->Play();
	}
}

void PlayerForm::RestoreFromSaveData(const SaveData& saveData)
{
	ChangeState(PlayerBase::PlayerType::TANUKI);

	PlayerBase* player = GetPlayer();
	if(!player)
	{
		return;
	}

	player->SetPos(saveData.playerPos);
	player->SetDir(saveData.playerDir);
	player->SetMakimonoCount(saveData.makimonoCount);
	player->_status = CharaBase::STATUS::WAIT;

	player->SetInputEnabled(true);
	player->Process();

	auto* tanuki = dynamic_cast<PlayerTanuki*>(player);
	if(tanuki)
	{
		tanuki->RestoreDefaultModel("idle", true);
		tanuki->ResetDash();
	}
}

void PlayerForm::TransformForEnemyDetetion(PlayerBase::PlayerType type, const vec::Vec3& pos, const vec::Vec3& dir)
{
	if(_playerType == type)
	{
		return;
	}

	PlayerBase* player = PlayerFactory::GetPlayer(type);
	if(!player)
	{
		return;
	}

	player->SetPos(pos);
	player->SetDir(dir);
	player->SetRotationY(atan2f(-dir.x, -dir.z));
	player->_status = CharaBase::STATUS::WAIT;

	player->PlayAnimation("idle", true);
	player->Process();

	_playerType = type;

	EffectManager::UpdatePalyerTransformEffect(player, true);
	EffectManager::UpdatePlayerPosition(player);

	auto soundFinish = gGlobal._soundServer->Get("3");
	if(soundFinish && !soundFinish->IsPlay())
	{
		soundFinish->Play();
	}
}