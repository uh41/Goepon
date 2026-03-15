#include "ObjectFactory.h"
#include "EnemySensor.h"
#include "Goal.h"
#include "player.h"

at::spc<EnemySensor> ObjectFactory::CreateEnemySensor(MapBase* map)
{
	auto sensor = std::make_shared<EnemySensor>();
	if(!sensor->Initialize())
	{
		return nullptr;
	}
	sensor->SetMap(map);
	return sensor;
}

std::string ObjectFactory::GetCustomId(const nlohmann::json& object)
{
	std::string gid;
	if(object.contains("customId"))
	{
		object.at("customId").get_to(gid);
	}
	return gid;
}

// --- 敵 / ステージ収集ロジック ---
bool ObjectFactory::CreateEnemyFromJson(
	const nlohmann::json& object,
	EnemyCreateContext& context
)
{
	if(!context.map || !context.enemyBase || !context.nextEnemyId)
	{
		return false;
	}

	const std::string& name = object.at("objectName");
	const std::string gid = GetCustomId(object);

	// 動かない敵
	if(name == "S_MarkerRX")
	{
		auto sensor = std::make_shared<EnemySensor>();
		if(!sensor->Initialize()) return false;
		sensor->SetMap(context.map);
		sensor->SetDetectionSector(400.0f, 120.0f);

		auto enemy = std::make_shared<Enemy>();
		enemy->Initialize();
		enemy->SetJsonDataUE(object);
		enemy->SetEnemySensor(sensor);
		enemy->SetEffect(context.hensinEffect);
		enemy->SetEffect(context.shirimochiEffect);
		enemy->SetStunEffect(context.stunEffect);
		enemy->SetEnemyId((*context.nextEnemyId)++);
		enemy->SetDirSequenceFromJson(object);

		context.enemyBase->emplace_back(std::move(enemy));
		return true;
	}

	// 動く敵
	if(name == "S_MarkerB")
	{
		auto sensor = std::make_shared<EnemySensor>();
		if(!sensor->Initialize()) return false;
		sensor->SetMap(context.map);
		sensor->SetDetectionSector(400.0f, 120.0f);

		auto enemyMove = std::make_shared<EnemyMove>();
		enemyMove->Initialize();
		enemyMove->SetJsonDataUE(object);
		enemyMove->SetEnemySensor(sensor);
		enemyMove->SetEffect(context.hensinEffect);
		enemyMove->SetEffect(context.shirimochiEffect);
		enemyMove->SetStunEffect(context.stunEffect);
		enemyMove->SetEnemyId((*context.nextEnemyId)++);
		enemyMove->SetDirSequenceFromJson(object);

		if(context.stageData)
		{
			auto itInfo = context.stageData->patrolPointInfo.find(gid);
			if(itInfo != context.stageData->patrolPointInfo.end() && !itInfo->second.empty())
			{
				enemyMove->SetPatrolPointInfo(itInfo->second);
				enemyMove->CaptureInitialTransform();
				context.enemyBase->emplace_back(std::move(enemyMove));
				return true;
			}
		}

		if(context.patrolGroup)
		{
			auto it = context.patrolGroup->find(gid);
			if(it != context.patrolGroup->end() && !it->second.empty())
			{
				enemyMove->SetPatrolPoint(it->second);
				enemyMove->CaptureInitialTransform();
			}
		}

		context.enemyBase->emplace_back(std::move(enemyMove));
		return true;
	}

	// 犬
	if(name == "Dog")
	{
		auto sensor = std::make_shared<EnemySensor>();
		if(!sensor->Initialize()) return false;
		sensor->SetMap(context.map);
		sensor->SetDetectionSector(380.0f, 80.0f);

		auto dog = std::make_shared<EnemyDog>();
		dog->Initialize();
		dog->SetJsonDataUE(object);
		dog->SetEnemySensor(sensor);
		dog->SetEffect(context.hensinEffect);
		dog->SetEnemyId((*context.nextEnemyId)++);

		if(context.dogMovementArea)
		{
			auto it = context.dogMovementArea->find(gid);
			if(it != context.dogMovementArea->end() && !it->second.empty())
			{
				dog->SetMovementArea(it->second);
			}
		}

		context.enemyBase->emplace_back(std::move(dog));
		return true;
	}

	return false;
}

bool ObjectFactory::CollectStageObjects(
	const ApplicationGlobal::StageData& stageData,
	StageLoadCollectContext& context
)
{
	if(
		!context.playerTanuki || !context.goal || !context.camera ||
		!context.treasureBase || !context.treasureRapidFire ||
		!context.makimono || !context.tutorial ||
		!context.enemyJsonList || !context.dogMovementAreas
		)
	{
		return false;
	}

	static const at::ust<std::string> skipObject = { "S_MarkerR" };
	static const at::ust<std::string> enemyObject = { "S_MarkerB", "S_MarkerRX", "Dog" };

	for(auto&& objData : stageData.object)
	{
		const std::string& name = objData.objectName;
		if(skipObject.find(name) != skipObject.end())
		{
			continue;
		}

		const nlohmann::json& object = objData.json;

		if(name == "S_MarkerDGR")
		{
			vec::Vec3 pos{};
			object.at("translate").at("x").get_to(pos.x);
			object.at("translate").at("y").get_to(pos.z);
			object.at("translate").at("z").get_to(pos.y);
			pos.z *= -1.0f;

			const std::string gid = GetCustomId(object);
			(*context.dogMovementAreas)[gid].push_back(pos);
			continue;
		}

		if(name == "S_MarkerA")
		{
			context.playerTanuki->SetJsonDataUE(object);
			continue;
		}

		if(enemyObject.count(name))
		{
			context.enemyJsonList->push_back(object);
			continue;
		}

		if(name == "Goal")
		{
			context.goal->SetJsonDataUE(object);
			continue;
		}

		if(name == "Treasure")
		{
			auto treasure = std::make_shared<Treasure>();
			treasure->Initialize();
			treasure->SetJsonDataUE(object);
			context.treasureBase->emplace_back(treasure);
			continue;
		}

		if(name == "TreasureX")
		{
			auto treasure = std::make_shared<TreasureRapidFire>();
			treasure->Initialize();
			treasure->SetJsonDataUE(object);
			context.treasureBase->emplace_back(treasure);
			context.treasureRapidFire->emplace_back(treasure);
			continue;
		}

		if(name == "Item")
		{
			auto makimono = std::make_shared<Makimono>();
			makimono->Initialize();
			makimono->SetJsonDataUE(object);
			makimono->SetCamera(context.camera);
			context.makimono->emplace_back(makimono);
			continue;
		}

		if(name == "S_Marker_Event")
		{
			auto tutorial = std::make_shared<Tutorial>();
			tutorial->Initialize();
			tutorial->SetJsonDataUE(object);

			if(object.contains("customId"))
			{
				std::string customId;
				object.at("customId").get_to(customId);
				tutorial->SetEventId(customId.empty() ? 0 : std::stoi(customId));
			}
			else
			{
				tutorial->SetEventId(0);
			}

			context.tutorial->emplace_back(tutorial);
			continue;
		}
	}

	return true;
}

// --- UI / エフェクト / その他生成関数 ---
at::spc<HenshinUi> ObjectFactory::CreateHenshinUi(void* owner)
{
	auto ui = std::make_shared<HenshinUi>();
	ui->SetOwner(owner);
	return ui;
}

at::spc<UiMakimono> ObjectFactory::CreateUiMakimono(Player* player)
{
	auto ui = std::make_shared<UiMakimono>();
	if(player) ui->SetPlayer(player);
	return ui;
}

at::spc<CounterUi> ObjectFactory::CreateCounterUi()
{
	return std::make_shared<CounterUi>();
}

at::spc<AttackUi> ObjectFactory::CreateAttackUi(Player* player)
{
	auto ui = std::make_shared<AttackUi>();
	if(player) ui->Show(player->GetPos());
	return ui;
}

at::spc<TreasureOpenUi> ObjectFactory::CreateTreasureOpenUi()
{
	return std::make_shared<TreasureOpenUi>();
}

at::spc<DashUi> ObjectFactory::CreateDashUi(PlayerTanuki* playerTanuki)
{
	auto ui = std::make_shared<DashUi>();
	if(playerTanuki) ui->SetPlayer(playerTanuki);
	return ui;
}

at::spc<TreasureUi> ObjectFactory::CreateTreasureUi(const at::vspc<Treasure>& treasureList)
{
	auto ui = std::make_shared<TreasureUi>();
	ui->SetTreasureList(treasureList);
	return ui;
}

at::spc<ConfigUi> ObjectFactory::CreateConfigUi(bool visible)
{
	auto ui = std::make_shared<ConfigUi>();
	ui->SetVisible(visible);
	return ui;
}

at::spc<TreasureEffect> ObjectFactory::CreateTreasureEffect()
{
	return std::make_shared<TreasureEffect>();
}

at::spc<HensinEffect> ObjectFactory::CreateHensinEffect()
{
	return std::make_shared<HensinEffect>();
}

at::spc<WalkEffect> ObjectFactory::CreateWalkEffect()
{
	return std::make_shared<WalkEffect>();
}

at::spc<FindEffect> ObjectFactory::CreateFindEffect()
{
	return std::make_shared<FindEffect>();
}

at::spc<HatenaEffect> ObjectFactory::CreateHatenaEffect()
{
	return std::make_shared<HatenaEffect>();
}

at::spc<DoyaEffect> ObjectFactory::CreateDoyaEffect()
{
	return std::make_shared<DoyaEffect>();
}

at::spc<NakiEffect> ObjectFactory::CreateNakiEffect()
{
	return std::make_shared<NakiEffect>();
}

at::spc<ShirimochiEffect> ObjectFactory::CreateShirimochiEffect()
{
	return std::make_shared<ShirimochiEffect>();
}

at::spc<StunEffect> ObjectFactory::CreateStunEffect()
{
	return std::make_shared<StunEffect>();
}

at::spc<SoundServer3D> ObjectFactory::CreateSoundServer3D(const at::spc<soundserver::SoundServer>& server, float radius)
{
	if(!server) return nullptr;
	auto s = std::make_shared<SoundServer3D>(server);
	s->SetRadius(radius);
	return s;
}

at::spc<Treasure> ObjectFactory::CreateTreasure()
{
	return std::make_shared<Treasure>();
}

at::spc<TreasureRapidFire> ObjectFactory::CreateTreasureRapidFire()
{
	return std::make_shared<TreasureRapidFire>();
}

at::spc<Makimono> ObjectFactory::CreateMakimono(Camera* camera)
{
	auto m = std::make_shared<Makimono>();
	if(camera) m->SetCamera(camera);
	return m;
}

at::spc<Tutorial> ObjectFactory::CreateTutorial()
{
	return std::make_shared<Tutorial>();
}

at::spc<Player> ObjectFactory::CreatePlayer()
{
	auto p = std::make_shared<Player>();
	return p;
}

at::spc<PlayerTanuki> ObjectFactory::CreatePlayerTanuki()
{
	auto p = std::make_shared<PlayerTanuki>();
	return p;
}

at::spc<PlayerMono> ObjectFactory::CreatePlayerMono()
{
	auto p = std::make_shared<PlayerMono>();
	return p;
}

at::spc<Goal> ObjectFactory::CreateGoal()
{
	auto g = std::make_shared<Goal>();
	return g;
}

at::spc<Camera> ObjectFactory::CreateCameraInitialized()
{
	auto cam = std::make_shared<Camera>();
	// Camera::Initialize() が存在する想定。存在しなければこの呼び出しは削除してください。
	if constexpr(std::is_invocable_r_v<bool, decltype(&Camera::Initialize), Camera*>)
	{
		if(!cam->Initialize())
		{
			return nullptr;
		}
	}
	return cam;
}

std::unique_ptr<CinematicCamera> ObjectFactory::CreateCinematicCameraInitialized()
{
	auto ccam = std::make_unique<CinematicCamera>();
	// CinematicCamera::Initialize() が存在する前提で初期化を試みる
	if(!ccam->Initialize())
	{
		return nullptr;
	}
	return ccam;
}