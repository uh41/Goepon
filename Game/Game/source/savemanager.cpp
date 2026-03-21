#include "savemanager.h"

bool SaveManager::JsonToVec3(const nlohmann::json& j, vec::Vec3& out)
{
	if(!j.is_object()) return false;
	if(!j.contains("x") || !j.contains("y") || !j.contains("z")) return false;
	out.x = j.at("x").get<float>();
	out.y = j.at("y").get<float>();
	out.z = j.at("z").get<float>();
	return true;
}

bool SaveManager::Save(const SaveData& data, const std::string& path)
{
	std::filesystem::path p(path);
	if(p.has_parent_path())
	{
		std::filesystem::create_directories(p.parent_path());// ディレクトリが存在しない場合は作成
	}

	nlohmann::json j;
	j["version"] = data.version;
	j["stageId"] = data.stageId;

	j["player"]["pos"] = Vec3ToJson(data.playerPos);
	j["player"]["dir"] = Vec3ToJson(data.playerDir);
	j["player"]["makimonoCount"] = data.makimonoCount;

	j["player"]["takenMakimonoIds"] = data.takenMakimonoIds;
	
	j["treasure"]["openId"] = data.openTreasureIds;

	j["enemys"] = nlohmann::json::array();
	for(auto& enemy : data.enemies)
	{
		nlohmann::json e;
		e["enemyId"] = enemy.enemyId;
		e["type"] = enemy.type;
		e["pos"] = Vec3ToJson(enemy.pos);
		e["dir"] = Vec3ToJson(enemy.dir);
		j["enemys"].push_back(e);
	}

	j["patrolGroups"] = nlohmann::json::object();
	for(auto& group : data.patrolGroups)
	{
		nlohmann::json g = nlohmann::json::array();
		for(auto& point : group.second)
		{
			nlohmann::json p;
			p["pos"] = Vec3ToJson(point.pos);
			p["id"] = point.id;
			p["waitTime"] = point.waitTime;
			g.push_back(p);
		}
		j["patrolGroups"][group.first] = g;
	}

	auto Path = p;
	Path += ".tmp"; // 書き込み中の一時ファイル
	{
		std::ofstream ofs(Path, std::ios::binary);// 書き込み用のファイルストリームを開く
		if(!ofs.is_open())
		{
			return false; // ファイルが開けない場合はfalseを返す
		}

		ofs << j.dump(2); // JSONを整形して書き込む
		ofs.close(); // ファイルストリームを閉じる
	}

	std::error_code ec; // エラーコードを受け取るためのオブジェクト
	std::filesystem::rename(Path, p, ec); // 一時ファイルを目的のファイル名にリネーム
	if(ec)
	{
		std::filesystem::remove(p, ec); // 既存のファイルを削除（存在する場合）
		std::filesystem::rename(Path, p, ec); // 再度リネームを試みる
	}

	return true;
}

bool SaveManager::TryLoad(SaveData& outData, const std::string& path)
{
	std::ifstream ifs(path, std::ios::binary);// 読み込み用のファイルストリームを開く
	if(!ifs.is_open())
	{
		return false; // ファイルが開けない場合はfalseを返す
	}

	nlohmann::json j;
	ifs >> j; // JSONを読み込む
	ifs.close(); // ファイルストリームを閉じる

	outData = SaveData{}; // デフォルト値で初期化

	if(j.contains("version"))
	{
		outData.version = j.at("version").get<int>();
	}
	if(j.contains("stageId"))
	{
		outData.stageId = j.at("stageId").get<std::string>();
	}

	if(j.contains("player"))
	{
		auto& player = j.at("player");
		if(player.contains("pos"))
		{
			JsonToVec3(player.at("pos"), outData.playerPos);
		}
		if(player.contains("dir"))
		{
			JsonToVec3(player.at("dir"), outData.playerDir);
		}
		if(player.contains("makimonoCount"))
		{
			outData.makimonoCount = player.at("makimonoCount").get<int>();
		}
		if(player.contains("takenMakimonoIds"))
		{
			outData.takenMakimonoIds = player.at("takenMakimonoIds").get<at::vet<int>>();
		}
	}

	if(j.contains("treasure") && j.at("treasure").contains("openId"))
	{
		outData.openTreasureIds = j.at("treasure").at("openId").get<at::vet<int>>();
	}

	if(j.contains("enemys") && j.at("enemys").is_array())
	{
		for(auto& e : j.at("enemys"))
		{
			SaveData::EnemyInitial enemy{};
			if(e.contains("enemyId"))
			{
				enemy.enemyId = e.at("enemyId").get<uint32_t>();
			}
			if(e.contains("type"))
			{
				enemy.type = e.at("type").get<std::string>();
			}
			if(e.contains("pos"))
			{
				JsonToVec3(e.at("pos"), enemy.pos);
			}
			if(e.contains("dir"))
			{
				JsonToVec3(e.at("dir"), enemy.dir);
			}
			outData.enemies.push_back(enemy);
		}
	}

	if(j.contains("patrolGroups") && j.at("patrolGroups").is_object())
	{
		for(auto it = j.at("patrolGroups").begin(); it != j.at("patrolGroups").end(); ++it)
		{
			std::string gid = it.key();
			auto& arr = it.value();
			if(!arr.is_array())
			{
				continue; // 配列でない場合はスキップ
			}

			at::vet<SaveData::PatrolPoint> points;
			for(auto& p : arr)
			{
				SaveData::PatrolPoint point{};
				if(p.contains("pos"))
				{
					JsonToVec3(p.at("pos"), point.pos);
				}
				if(p.contains("id"))
				{
					point.id = p.at("id").get<int>();
				}
				if(p.contains("waitTime"))
				{
					point.waitTime = p.at("waitTime").get<float>();
				}
				points.push_back(std::move(point));
			}
			outData.patrolGroups[gid] = std::move(points);
		}
	}

	return true;
}