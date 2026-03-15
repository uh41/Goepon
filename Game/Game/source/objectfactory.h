#pragma once

#include "appframe.h"
#include "applicationglobal.h"
#include "MapBase.h"

// UI
#include "henshinui.h"
#include "UiMakimono.h"
#include "counterui.h"
#include "attackui.h"
#include "treasureopenui.h"
#include "dashui.h"
#include "treasureui.h"
#include "configui.h"

// エフェクト
#include "treasureeffect.h"
#include "hensineffect.h"
#include "walkeffect.h"
#include "findeffect.h"
#include "hatenaeffect.h"
#include "doyaeffect.h"
#include "nakieffect.h"
#include "shirimochieffect.h"
#include "stuneffect.h"

// 敵関連
#include "enemysensor.h"
#include "enemy.h"
#include "enemymove.h"
#include "enemydog.h"

// アイテム等
#include "Treasure.h"
#include "TreasureRapidFire.h"
#include "Makimono.h"
#include "tutorial.h"

// カメラ / サウンド
#include "camera.h"
#include "soundserver3D.h"

// プレイヤー系（利用設定用）
#include "player.h"
#include "playertanuki.h"
#include "playermono.h"
#include "Goal.h"

#include "camera.h"
#include "CinematicCamera.h"

class ObjectFactory
{
public:
	using EnemyGroup = at::umtt<std::string, at::vet<vec::Vec3>>;

	// 敵生成に必要なコンテキスト
	struct EnemyCreateContext
	{
		MapBase* map = nullptr;
		const ApplicationGlobal::StageData* stageData = nullptr;
		at::spc<HensinEffect> hensinEffect;           // raw pointer -> shared_ptr に変更
		at::spc<ShirimochiEffect> shirimochiEffect;   // 同上
		at::spc<StunEffect> stunEffect;               // 同上
		at::vspc<EnemyBase>* enemyBase = nullptr;
		uint32_t* nextEnemyId = nullptr;
		const EnemyGroup* patrolGroup = nullptr;
		const EnemyGroup* dogMovementArea = nullptr;
	};

	// LoadStageData での収集用コンテキスト
	struct StageLoadCollectContext
	{
		PlayerTanuki* playerTanuki = nullptr;
		Goal* goal = nullptr;
		Camera* camera = nullptr;

		at::vspc<TreasureBase>* treasureBase = nullptr;
		at::vspc<TreasureRapidFire>* treasureRapidFire = nullptr;
		at::vspc<Makimono>* makimono = nullptr;
		at::vspc<Tutorial>* tutorial = nullptr;

		at::vet<nlohmann::json>* enemyJsonList = nullptr;
		EnemyGroup* dogMovementAreas = nullptr;
		const ApplicationGlobal::StageData* stageData = nullptr;
	};

	// 敵 JSON から敵インスタンスを作成し、context->enemyBase に追加する
	static bool CreateEnemyFromJson(
		const nlohmann::json& object,
		EnemyCreateContext& context
	);

	// ステージ JSON を走査して各種オブジェクト（宝箱・巻物・敵 JSON 収集など）を収集する
	static bool CollectStageObjects(
		const ApplicationGlobal::StageData& stageData,
		StageLoadCollectContext& context
	);

	static at::spc<Player> CreatePlayer();
	static at::spc<PlayerTanuki> CreatePlayerTanuki();
	static at::spc<PlayerMono> CreatePlayerMono();
	static at::spc<Goal> CreateGoal();

	// カメラ生成（初期化済みを返す）
		// カメラ生成（初期化済みを返す）
	static at::spc<Camera> CreateCameraInitialized();
	// CinematicCamera は unique_ptr で所有するように変更
	static std::unique_ptr<CinematicCamera> CreateCinematicCameraInitialized();

	// UI の生成
	static at::spc<HenshinUi> CreateHenshinUi(void* owner);
	static at::spc<UiMakimono> CreateUiMakimono(Player* player);
	static at::spc<CounterUi> CreateCounterUi();
	static at::spc<AttackUi> CreateAttackUi(Player* player);
	static at::spc<TreasureOpenUi> CreateTreasureOpenUi();
	static at::spc<DashUi> CreateDashUi(PlayerTanuki* playerTanuki);
	static at::spc<TreasureUi> CreateTreasureUi(const at::vspc<Treasure>& treasureList);
	static at::spc<ConfigUi> CreateConfigUi(bool visible);

	// エフェクトの生成
	static at::spc<TreasureEffect> CreateTreasureEffect();
	static at::spc<HensinEffect> CreateHensinEffect();
	static at::spc<WalkEffect> CreateWalkEffect();
	static at::spc<FindEffect> CreateFindEffect();
	static at::spc<HatenaEffect> CreateHatenaEffect();
	static at::spc<DoyaEffect> CreateDoyaEffect();
	static at::spc<NakiEffect> CreateNakiEffect();
	static at::spc<ShirimochiEffect> CreateShirimochiEffect();
	static at::spc<StunEffect> CreateStunEffect();

	// サウンド3D
	static at::spc<SoundServer3D> CreateSoundServer3D(const at::spc<soundserver::SoundServer>& server, float radius = 768.0f);

	// 敵センサー（ヘルパー）
	static at::spc<EnemySensor> CreateEnemySensor(MapBase* map);

	// アイテム等
	static at::spc<Treasure> CreateTreasure();
	static at::spc<TreasureRapidFire> CreateTreasureRapidFire();
	static at::spc<Makimono> CreateMakimono(Camera* camera);
	static at::spc<Tutorial> CreateTutorial();

private:
	ObjectFactory() = delete;
	static std::string GetCustomId(const nlohmann::json& object);
};