/*********************************************************************/
// * \file   ObjectFactory.h
// * \brief  オブジェクト生成ファクトリクラス
// *
// * \author 鈴木裕稀
// * \date   2026/03/13
// * \作業内容: 新規作成 鈴木裕稀　2026/03/13
/*********************************************************************/

#pragma once
#include "appframe.h"
#include "objectbase.h"
#include "charabase.h"
#include "playerbase.h"
#include "enemybase.h"
#include "treasurebase.h"
#include "uibase.h"
#include "effectbase.h"

// 前方宣言
class MapBase;
class Camera;
class ModeGame;

class ObjectFactory
{
public:
	ObjectFactory();
	~ObjectFactory();

	// プレイヤー生成・初期化
	at::spc<PlayerBase> CreateAndInitializePlayer();
	at::spc<PlayerBase> CreateAndInitializePlayerTanuki();
	at::spc<PlayerBase> CreateAndInitializePlayerMono();

	// 敵生成・初期化
	at::spc<EnemyBase> CreateAndInitializeEnemy(const nlohmann::json& object,
		const at::umtt<std::string, at::vet<vec::Vec3>>& patrolGroup,
		const at::umtt<std::string, at::vet<vec::Vec3>>& dogMovementArea,
		uint32_t& nextEnemyId,
		MapBase* map,
		Camera* cam);

	at::spc<EnemyBase> CreateAndInitializeEnemyMove(const nlohmann::json& object,
		const at::umtt<std::string, at::vet<vec::Vec3>>& patrolGroup,
		MapBase* map,
		Camera* cam);

	at::spc<EnemyBase> CreateAndInitializeEnemyDog(const nlohmann::json& object,
		const at::umtt<std::string, at::vet<vec::Vec3>>& dogMovementArea,
		uint32_t& nextEnemyId,
		MapBase* map,
		Camera* cam);

	// 敵センサー生成・初期化
	at::spc<EnemyBase> CreateAndInitializeEnemySensor(float soundArea, MapBase* map);

	// 宝箱生成・初期化
	at::spc<TreasureBase> CreateAndInitializeTreasure(const nlohmann::json& object, MapBase* map, Camera* cam);
	at::spc<TreasureBase> CreateAndInitializeTreasureRapidFire(const nlohmann::json& object, MapBase* map, Camera* cam);

	// UI生成・初期化
	at::spc<UiBase> CreateAndInitializeHenshinUi(ModeGame* owner);
	at::spc<UiBase> CreateAndInitializeUiMakimono(PlayerBase* player);
	at::spc<UiBase> CreateAndInitializeCounterUi();
	at::spc<UiBase> CreateAndInitializeAttackUi(const vec::Vec3& pos);
	at::spc<UiBase> CreateAndInitializeTreasureOpenUi();
	at::spc<UiBase> CreateAndInitializeDashUi(PlayerBase* player);
	at::spc<UiBase> CreateAndInitializeTreasureUi(const at::vec<at::spc<TreasureBase>>& treasures);
	at::spc<UiBase> CreateAndInitializeConfigUi();

	// エフェクト生成・初期化
	at::spc<EffectBase> CreateAndInitializeTreasureEffect();
	at::spc<EffectBase> CreateAndInitializeHensinEffect();
	at::spc<EffectBase> CreateAndInitializeWalkEffect();
	at::spc<EffectBase> CreateAndInitializeFindEffect();
	at::spc<EffectBase> CreateAndInitializeHatenaEffect();
	at::spc<EffectBase> CreateAndInitializeDoyaEffect(PlayerBase* player);
	at::spc<EffectBase> CreateAndInitializeNakiEffect(PlayerBase* player);
	at::spc<EffectBase> CreateAndInitializeShirimochiEffect();
	at::spc<EffectBase> CreateAndInitializeStunEffect();

	// オブジェクト生成・初期化
	at::spc<ObjectBase> CreateAndInitializeGoal(Camera* cam);
	at::spc<ObjectBase> CreateAndInitializeMap1(Camera* cam);
	at::spc<ObjectBase> CreateAndInitializeCube(Camera* cam);

	// シャドウ生成・初期化
	at::spc<CharaBase> CreateAndInitializeCharaShadow(CharaBase* target);

	// 巻物生成・初期化
	at::spc<CharaBase> CreateAndInitializeMakimono(Camera* cam);

private:
};
