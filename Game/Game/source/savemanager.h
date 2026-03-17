#pragma once
#include "appframe.h"

struct SaveData
{
	int version = 1;

	std::string stageId; // ステージID

	// プレイヤーの状態
	vec::Vec3 playerPos;
	vec::Vec3 playerDir;

	int makimonoCount; // 巻物の所持数

	at::vet<int> openTreasureIds; // 開けた宝箱のIDリスト

	struct EnemyInitial
	{
		uint32_t enemyId; // 敵のID
		std::string type;    // 敵の種類
		vec::Vec3 pos;    // 敵の初期位置
		vec::Vec3 dir;    // 敵の初期向き
	};

	struct PatrolPoint
	{
		vec::Vec3 pos; // 巡回点の位置
		int id;       // 巡回点のID（敵の向きなどに使う）
		float waitTime; // 巡回点での待機時間
	};

	at::vet<EnemyInitial> enemies; // 敵の初期状態リスト
	at::umtt<std::string, at::vet<PatrolPoint>> patrolGroups; // 巡回グループごとの巡回点リスト
};


class SaveManager
{
public:
	static std::string GetDefaultPath()
	{
		return "res/save/saveData.json";
	}

	static nlohmann::json Vec3ToJson(const vec::Vec3& v)
	{
		return nlohmann::json
		{
			{"x", v.x},
			{"y", v.y},
			{"z", v.z}
		};
	}

	static bool JsonToVec3(const nlohmann::json& j, vec::Vec3& out);
	static bool Save(const SaveData& data, const std::string& path = GetDefaultPath());
	static bool TryLoad(SaveData& outData, const std::string& path = GetDefaultPath());

private:
	SaveManager() = default;
	~SaveManager() = default;
};

