#include "effectmanager.h"
#include "playerbase.h"
#include "Goal.h"
#include "TreasureBase.h"
#include "enemybase.h"

at::upc<WalkEffect> EffectManager::_walkEffect = nullptr;
at::upc<DoyaEffect> EffectManager::_doyaEffect = nullptr;
at::upc<NakiEffect> EffectManager::_nakiEffect = nullptr;
at::upc<ShirimochiEffect> EffectManager::_shirimochiEffect = nullptr;
at::upc<SavePointEffect> EffectManager::_savePointEffect = nullptr;
at::upc<GoalEffect> EffectManager::_goalEffect = nullptr;
at::upc<TreasureEffect> EffectManager::_treasureEffect = nullptr;
at::upc<EffectPool<HensinEffect>> EffectManager::_hensinEffectPool = nullptr;
at::upc<EffectPool<FindEffect>> EffectManager::_findEffectPool = nullptr;
at::upc<EffectPool<HatenaEffect>> EffectManager::_hatenaEffectPool = nullptr;
at::upc<EffectPool<StunEffect>> EffectManager::_stunEffectPool = nullptr;
at::upc<EffectPool<MakimonoGetEffect>> EffectManager::_makimonoGetEffectPool = nullptr;

bool EffectManager::Initialize()
{
	if(!InitializeSingletonEffect())
	{
		return false;
	}
	if(!InitializePooledEffect())
	{
		return false;
	}
	if(!InitializeMultiInstanceEffect())
	{
		return false;
	}
	return true;
}

bool EffectManager::Terminate()
{
	if(!TerminateSingletonEffect())
	{
		return false;
	}
	if(!TerminatePooledEffect())
	{
		return false;
	}
	if(!TerminateMultiInstanceEffect())
	{
		return false;
	}
	return true;
}

bool EffectManager::InitializeSingletonEffect()
{
	_walkEffect = std::make_unique<WalkEffect>();
	if(!_walkEffect->Initialize())
	{
		return false;
	}
	_doyaEffect = std::make_unique<DoyaEffect>();
	if(!_doyaEffect->Initialize())
	{
		return false;
	}
	_nakiEffect = std::make_unique<NakiEffect>();
	if(!_nakiEffect->Initialize())
	{
		return false;
	}
	_shirimochiEffect = std::make_unique<ShirimochiEffect>();
	if(!_shirimochiEffect->Initialize())
	{
		return false;
	}
	_savePointEffect = std::make_unique<SavePointEffect>();
	if(!_savePointEffect->Initialize())
	{
		return false;
	}
	_goalEffect = std::make_unique<GoalEffect>();
	if(!_goalEffect->Initialize())
	{
		return false;
	}
	_treasureEffect = std::make_unique<TreasureEffect>();
	if(!_treasureEffect->Initialize())
	{
		return false;
	}
	return true;
}

bool EffectManager::InitializeMultiInstanceEffect()
{
	_treasureEffect = std::make_unique<TreasureEffect>();
	if(!_treasureEffect->Initialize())
	{
		return false;
	}
	return true;
}

bool EffectManager::InitializePooledEffect()
{
	_hensinEffectPool = std::make_unique<EffectPool<HensinEffect>>(10); // 変身エフェクトを事前に登録
	_findEffectPool = std::make_unique<EffectPool<FindEffect>>(10); // 発見エフェクトを事前に登録
	_hatenaEffectPool = std::make_unique<EffectPool<HatenaEffect>>(10); // はてなエフェクトを事前に登録
	_stunEffectPool = std::make_unique<EffectPool<StunEffect>>(10); // スタンエフェクトを事前に登録
	_makimonoGetEffectPool = std::make_unique<EffectPool<MakimonoGetEffect>>(10); // 巻物ゲットエフェクトを事前に登録
	return true;
}

bool EffectManager::TerminateSingletonEffect()
{
	if(_walkEffect)
	{
		_walkEffect->Terminate();
		_walkEffect.reset();
	}
	if(_doyaEffect)
	{
		_doyaEffect->Terminate();
		_doyaEffect.reset();
	}
	if(_nakiEffect)
	{
		_nakiEffect->Terminate();
		_nakiEffect.reset();
	}
	if(_shirimochiEffect)
	{
		_shirimochiEffect->Terminate();
		_shirimochiEffect.reset();
	}
	if(_savePointEffect)
	{
		_savePointEffect->Terminate();
		_savePointEffect.reset();
	}
	if(_goalEffect)
	{
		_goalEffect->Terminate();
		_goalEffect.reset();
	}
	if(_treasureEffect)
	{
		_treasureEffect->Terminate();
		_treasureEffect.reset();
	}
	return true;
}

bool EffectManager::TerminateMultiInstanceEffect()
{
	if(_treasureEffect)
	{
		_treasureEffect->Terminate();
		_treasureEffect.reset();
	}
	return true;
}

bool EffectManager::TerminatePooledEffect()
{
	if(_hensinEffectPool)
	{
		_hensinEffectPool->Terminate();
		_hensinEffectPool.reset();
	}
	if(_findEffectPool)
	{
		_findEffectPool->Terminate();
		_findEffectPool.reset();
	}
	if(_hatenaEffectPool)
	{
		_hatenaEffectPool->Terminate();
		_hatenaEffectPool.reset();
	}
	if(_stunEffectPool)
	{
		_stunEffectPool->Terminate();
		_stunEffectPool.reset();
	}
	if(_makimonoGetEffectPool)
	{
		_makimonoGetEffectPool->Terminate();
		_makimonoGetEffectPool.reset();
	}
	return true;
}

WalkEffect* EffectManager::GetWalkEffect()
{
	return _walkEffect.get();
}

DoyaEffect* EffectManager::GetDoyaEffect()
{
	return _doyaEffect.get();
}

NakiEffect* EffectManager::GetNakiEffect()
{
	return _nakiEffect.get();
}

ShirimochiEffect* EffectManager::GetShirimochiEffect()
{
	return _shirimochiEffect.get();
}

SavePointEffect* EffectManager::GetSavePointEffect()
{
	return _savePointEffect.get();
}

GoalEffect* EffectManager::GetGoalEffect()
{
	return _goalEffect.get();
}

TreasureEffect* EffectManager::GetTreasureEffect()
{
	return _treasureEffect.get();
}

HensinEffect* EffectManager::PlayeHensinEffect(const vec::Vec3& pos)
{
	if(!_hensinEffectPool)
	{
		return nullptr;
	}

	auto effect = _hensinEffectPool->Acquire();
	if(effect)
	{
		effect->PlayEffect(pos);
	}
	return effect;
}

FindEffect* EffectManager::PlayFindEffect(const vec::Vec3& pos)
{
	if(!_findEffectPool)
	{
		return nullptr;
	}
	auto effect = _findEffectPool->Acquire();
	if(effect)
	{
		effect->PlayEffect(pos);
	}
	return effect;
}

HatenaEffect* EffectManager::PlayHatenaEffect(EnemyBase* enemy)
{
	if(!_hatenaEffectPool || !enemy)
	{
		return nullptr;
	}
	auto effect = _hatenaEffectPool->Acquire();
	if(effect)
	{
		effect->PlayOnce(enemy);
	}
	return effect;
}

StunEffect* EffectManager::PlayStunEffect(const vec::Vec3& pos)
{
	if(!_stunEffectPool)
	{
		return nullptr;
	}
	auto effect = _stunEffectPool->Acquire();
	if(effect)
	{
		effect->PlayEffect(pos);
	}
	return effect;
}

MakimonoGetEffect* EffectManager::PlayMakimonoGetEffect(const vec::Vec3& pos)
{
	if(!_makimonoGetEffectPool)
	{
		return nullptr;
	}
	auto effect = _makimonoGetEffectPool->Acquire();
	if(effect)
	{
		effect->PlayEffect(pos);
	}
	return effect;
}

void EffectManager::UpdatePlayerPosition(PlayerBase* player)
{
	if(!player)
	{
		return;
	}

	if(_walkEffect)
	{
		_walkEffect->SetPlayerPos(player);
	}
	if(_doyaEffect)
	{
		_doyaEffect->SetTargetPlayer(player);
	}
	if(_nakiEffect)
	{
		_nakiEffect->SetTargetPlayer(player);
	}
	if(_savePointEffect)
	{
		_savePointEffect->SetTargetPlayer(player);
	}
}

void EffectManager::UpdatePalyerTransformEffect(PlayerBase* player, bool playTransform)
{
	if(!player)
	{
		return;
	}
	if(playTransform)
	{
		PlayeHensinEffect(player->GetPos());
	}

	UpdatePlayerPosition(player);
}

void EffectManager::SetGoal(at::spc<Goal> goal)
{
	if(_goalEffect)
	{
		_goalEffect->SetGoal(goal);
	}
}

void EffectManager::SetInitialPlayer(PlayerBase* player)
{
	if(!player)
	{
		return;
	}
	if(_doyaEffect)
	{
		_doyaEffect->SetTargetPlayer(player);
	}
	if(_nakiEffect)
	{
		_nakiEffect->SetTargetPlayer(player);
	}
}

void EffectManager::SetTreasure(const at::vspc<TreasureBase>& treasure)
{
	if(_treasureEffect)
	{
		_treasureEffect->SetTreasure(treasure);
	}
}

void EffectManager::UpdateAllEffect()
{
	if(_walkEffect)
	{
		_walkEffect->Process();
	}
	if(_doyaEffect)
	{
		_doyaEffect->Process();
	}
	if(_nakiEffect)
	{
		_nakiEffect->Process();
	}
	if(_shirimochiEffect)
	{
		_shirimochiEffect->Process();
	}
	if(_savePointEffect)
	{
		_savePointEffect->Process();
	}
	if(_goalEffect)
	{
		_goalEffect->Process();
	}
	if(_treasureEffect)
	{
		_treasureEffect->Process();
	}

	if(_hensinEffectPool)
	{
		_hensinEffectPool->Update();
	}
	if(_findEffectPool)
	{
		_findEffectPool->Update();
	}
	if(_hatenaEffectPool)
	{
		_hatenaEffectPool->Update();
	}
	if(_stunEffectPool)
	{
		_stunEffectPool->Update();
	}
	if(_makimonoGetEffectPool)
	{
		_makimonoGetEffectPool->Update();
	}
}