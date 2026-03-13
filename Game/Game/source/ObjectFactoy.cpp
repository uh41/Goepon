/*********************************************************************/
// * \file   ObjectFactory.cpp
// * \brief  オブジェクト生成ファクトリクラス
// *
// * \author 鈴木裕稀
// * \date   2026/03/13
// * \作業内容: 新規作成 鈴木裕稀　2026/03/13
/*********************************************************************/

#include "ObjectFactoy.h"
#include "player.h"
#include "playertanuki.h"
#include "playermono.h"
#include "enemy.h"
#include "enemymove.h"
#include "enemydog.h"
#include "enemysensor.h"
#include "treasure.h"
#include "TreasureRapidFire.h"
#include "henshinui.h"
#include "UiMakimono.h"
#include "counterui.h"
#include "attackui.h"
#include "treasureopenui.h"
#include "dashui.h"
#include "treasureui.h"
#include "configui.h"
#include "treasureeffect.h"
#include "hensineffect.h"
#include "walkeffect.h"
#include "findeffect.h"
#include "hatenaeffect.h"
#include "doyaeffect.h"
#include "nakieffect.h"
#include "shirimochieffect.h"
#include "stuneffect.h"
#include "Goal.h"
#include "Map1.h"
#include "cube.h"
#include "charashadow.h"
#include "Makimono.h"
#include "modegame.h"

ObjectFactory::ObjectFactory()
{
}

ObjectFactory::~ObjectFactory()
{
}

at::spc<PlayerBase> ObjectFactory::CreateAndInitializePlayer()
{
	auto player = std::make_shared<Player>();
	player->Initialize();
	return player;
}

at::spc<PlayerBase> ObjectFactory::CreateAndInitializePlayerTanuki()
{
	auto player = std::make_shared<PlayerTanuki>();
	player->Initialize();
	return player;
}

at::spc<PlayerBase> ObjectFactory::CreateAndInitializePlayerMono()
{
	auto player = std::make_shared<PlayerMono>();
	player->Initialize();
	return player;
}

at::spc<EnemyBase> ObjectFactory::CreateAndInitializeEnemy(const nlohmann::json& object,
	const at::umtt<std::string, at::vet<vec::Vec3>>& patrolGroup,
	const at::umtt<std::string, at::vet<vec::Vec3>>& dogMovementArea,
	uint32_t& nextEnemyId,
	MapBase* map,
	Camera* cam)
{
	auto enemy = std::make_shared<Enemy>();
	enemy->Initialize();
	enemy->SetCamera(cam);
	return enemy;
}

at::spc<EnemyBase> ObjectFactory::CreateAndInitializeEnemyMove(const nlohmann::json& object,
	const at::umtt<std::string, at::vet<vec::Vec3>>& patrolGroup,
	MapBase* map,
	Camera* cam)
{
	auto enemy = std::make_shared<EnemyMove>();
	enemy->Initialize();
	enemy->SetCamera(cam);
	return enemy;
}

at::spc<EnemyBase> ObjectFactory::CreateAndInitializeEnemyDog(const nlohmann::json& object,
	const at::umtt<std::string, at::vet<vec::Vec3>>& dogMovementArea,
	uint32_t& nextEnemyId,
	MapBase* map,
	Camera* cam)
{
	auto enemy = std::make_shared<EnemyDog>();
	enemy->Initialize();
	enemy->SetCamera(cam);
	return enemy;
}

at::spc<EnemyBase> ObjectFactory::CreateAndInitializeEnemySensor(float soundArea, MapBase* map)
{
	auto sensor = std::make_shared<EnemySensor>();
	sensor->Initialize();
	return sensor;
}

at::spc<TreasureBase> ObjectFactory::CreateAndInitializeTreasure(const nlohmann::json& object, MapBase* map, Camera* cam)
{
	auto treasure = std::make_shared<Treasure>();
	treasure->Initialize();
	treasure->SetCamera(cam);
	return treasure;
}

at::spc<TreasureBase> ObjectFactory::CreateAndInitializeTreasureRapidFire(const nlohmann::json& object, MapBase* map, Camera* cam)
{
	auto treasure = std::make_shared<TreasureRapidFire>();
	treasure->Initialize();
	treasure->SetCamera(cam);
	return treasure;
}

at::spc<UiBase> ObjectFactory::CreateAndInitializeHenshinUi(ModeGame* owner)
{
	auto ui = std::make_shared<HenshinUi>();
	ui->Initialize();
	ui->SetOwner(owner);
	return ui;
}

at::spc<UiBase> ObjectFactory::CreateAndInitializeUiMakimono(PlayerBase* player)
{
	auto ui = std::make_shared<UiMakimono>();
	ui->Initialize();
	ui->SetPlayer(player);
	return ui;
}

at::spc<UiBase> ObjectFactory::CreateAndInitializeCounterUi()
{
	auto ui = std::make_shared<CounterUi>();
	ui->Initialize();
	return ui;
}

at::spc<UiBase> ObjectFactory::CreateAndInitializeAttackUi(const vec::Vec3& pos)
{
	auto ui = std::make_shared<AttackUi>();
	ui->Initialize();
	ui->Show(pos);
	return ui;
}

at::spc<UiBase> ObjectFactory::CreateAndInitializeTreasureOpenUi()
{
	auto ui = std::make_shared<TreasureOpenUi>();
	ui->Initialize();
	return ui;
}

at::spc<UiBase> ObjectFactory::CreateAndInitializeDashUi(PlayerBase* player)
{
	auto ui = std::make_shared<DashUi>();
	ui->Initialize();
	ui->SetPlayer(dynamic_cast<PlayerTanuki*>(player));
	return ui;
}

at::spc<UiBase> ObjectFactory::CreateAndInitializeTreasureUi(const at::vec<at::spc<TreasureBase>>& treasures)
{
	auto ui = std::make_shared<TreasureUi>();
	ui->Initialize();

	// TreasureBase から Treasure へのダウンキャストを行い、リストを作成
	at::vspc<Treasure> treasureList;
	treasureList.reserve(treasures.size());
	for(const auto& t : treasures)
	{
		if(auto casted = std::dynamic_pointer_cast<Treasure>(t))
		{
			treasureList.push_back(casted);
		}
	}
	ui->SetTreasureList(treasureList);
	return ui;
	return ui;
}

at::spc<UiBase> ObjectFactory::CreateAndInitializeConfigUi()
{
	auto ui = std::make_shared<ConfigUi>();
	ui->Initialize();
	ui->SetVisible(true);
	return ui;
}

at::spc<EffectBase> ObjectFactory::CreateAndInitializeTreasureEffect()
{
	auto effect = std::make_shared<TreasureEffect>();
	effect->Initialize();
	return effect;
}

at::spc<EffectBase> ObjectFactory::CreateAndInitializeHensinEffect()
{
	auto effect = std::make_shared<HensinEffect>();
	effect->Initialize();
	return effect;
}

at::spc<EffectBase> ObjectFactory::CreateAndInitializeWalkEffect()
{
	auto effect = std::make_shared<WalkEffect>();
	effect->Initialize();
	return effect;
}

at::spc<EffectBase> ObjectFactory::CreateAndInitializeFindEffect()
{
	auto effect = std::make_shared<FindEffect>();
	effect->Initialize();
	return effect;
}

at::spc<EffectBase> ObjectFactory::CreateAndInitializeHatenaEffect()
{
	auto effect = std::make_shared<HatenaEffect>();
	effect->Initialize();
	return effect;
}

at::spc<EffectBase> ObjectFactory::CreateAndInitializeDoyaEffect(PlayerBase* player)
{
	auto effect = std::make_shared<DoyaEffect>();
	effect->Initialize();
	effect->SetTargetPlayer(player);
	return effect;
}

at::spc<EffectBase> ObjectFactory::CreateAndInitializeNakiEffect(PlayerBase* player)
{
	auto effect = std::make_shared<NakiEffect>();
	effect->Initialize();
	effect->SetTargetPlayer(player);
	return effect;
}

at::spc<EffectBase> ObjectFactory::CreateAndInitializeShirimochiEffect()
{
	auto effect = std::make_shared<ShirimochiEffect>();
	effect->Initialize();
	return effect;
}

at::spc<EffectBase> ObjectFactory::CreateAndInitializeStunEffect()
{
	auto effect = std::make_shared<StunEffect>();
	effect->Initialize();
	return effect;
}

at::spc<ObjectBase> ObjectFactory::CreateAndInitializeGoal(Camera* cam)
{
	auto goal = std::make_shared<Goal>();
	goal->Initialize();
	goal->SetCamera(cam);
	return goal;
}

at::spc<ObjectBase> ObjectFactory::CreateAndInitializeMap1(Camera* cam)
{
	auto map = std::make_shared<Map1>();
	map->Initialize();
	map->SetCamera(cam);
	return map;
}

at::spc<ObjectBase> ObjectFactory::CreateAndInitializeCube(Camera* cam)
{
	auto cube = std::make_shared<Cube>();
	cube->Initialize();
	cube->SetCamera(cam);
	return cube;
}

at::spc<CharaBase> ObjectFactory::CreateAndInitializeCharaShadow(CharaBase* target)
{
	auto shadow = std::make_shared<CharaShadow>();
	shadow->Initialize();
	return shadow;
}

at::spc<CharaBase> ObjectFactory::CreateAndInitializeMakimono(Camera* cam)
{
	auto makimono = std::make_shared<Makimono>();
	makimono->Initialize();
	makimono->SetCamera(cam);
	return makimono;
}