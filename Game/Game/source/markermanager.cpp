/*********************************************************************/
// * \file   markermanager.h
// * \brief  マーカー（スポーン位置など）管理クラス
// *
// * \author 鈴木裕稀
/*********************************************************************/

#include "markermanager.h"

at::umtc<std::string, MarkerManager::MarkerData> MarkerManager::_marker; // マーカーのデータマップ
at::umtc<MarkerManager::MarkerType, at::vec<MarkerManager::MarkerData>> MarkerManager::_markerType; // マーカータイプとマーカーデータのリストのマップ

bool MarkerManager::Initialize()
{
	ClearAllMarker(); // 既存のマーカーデータをクリア
	return true;
}

MarkerManager::MarkerType MarkerManager::GetMarkerType(const std::string& objectName)
{
	if (objectName == "S_MarkerA")
	{
		return MarkerType::PLAYER_SPAWN;
	}
	if (objectName == "S_MarkerRX")
	{
		return MarkerType::ENEMY_STATIC;
	}
	if (objectName == "S_MarkerB")
	{
		return MarkerType::ENEMY_MOVE;
	}
	if (objectName == "Dog")
	{
		return MarkerType::ENEMY_DOG;
	}
	return MarkerType::UNKNOWN; // 不明なマーカータイプ
}

MarkerManager::MarkerData MarkerManager::ParseMarkerFromJson(const nlohmann::json& object)
{
	MarkerData data;
	if(object.contains("objectName"))
	{
		object.at("objectName").get_to(data.objectName);
		data.type = GetMarkerType(data.objectName);
	}

	if(object.contains("customId"))
	{
		object.at("customId").get_to(data.customId);
	}

	if(object.contains("translate"))
	{
		object.at("translate").at("x").get_to(data.pos.x);
		object.at("translate").at("y").get_to(data.pos.z); // UE:y -> DXLib:z
		object.at("translate").at("z").get_to(data.pos.y); // UE:z -> DXLib:y
		data.pos.z *= -1.0f;
	}

	if(object.contains("rotate"))
	{
		auto& rotate = object.at("rotate");
		float rotXDeg = rotate.at("x").get<float>();
		float rotYDeg = rotate.at("y").get<float>();
		float rotZDeg = rotate.at("z").get<float>();

		data.rotation = DEG2RAD(rotYDeg); // Y軸回転をラジアンに変換して保存
	}

	data.jsonData = object;

	return data;
}

const MarkerManager::MarkerData* MarkerManager::GetMarkerData(const std::string& customId)
{
	auto it = _marker.find(customId);
	if(it != _marker.end())
	{
		return &(it->second);
	}
	return nullptr; // 見つからない場合はnullptrを返す
}

void MarkerManager::RegisterMarkerData(const MarkerData& data)
{
	if(data.customId.empty())
	{
		return; // カスタムIDが空の場合は登録しない
	}

	_marker[data.customId] = data; // カスタムIDをキーにしてマーカーデータを登録
	std::string typeKey = std::to_string(static_cast<int>(data.type)); // マーカータイプを文字列キーに変換
	_markerType[data.type].push_back(data); // マーカータイプをキーにしてマーカーデータをリストに追加
}

void MarkerManager::ClearAllMarker()
{
	_marker.clear(); // マーカーデータマップをクリア
	_markerType.clear(); // マーカータイプマップをクリア
}

at::vec<MarkerManager::MarkerData> MarkerManager::GetMarkerType(MarkerType type)
{
	std::string typeKey = std::to_string(static_cast<int>(type)); // マーカータイプを文字列キーに変換
	auto it = _markerType.find(type);
	if(it != _markerType.end())
	{
		return it->second; // マーカータイプに対応するマーカーデータのリストを返す
	}
	return at::vec<MarkerManager::MarkerData>(); // 見つからない場合は空のリストを返す
}

float MarkerManager::GetPlayerSpawnRotation()
{
	auto marker = GetMarkerType(MarkerType::PLAYER_SPAWN);
	if(!marker.empty())
	{
		return marker[0].rotation; // プレイヤースポーンマーカーの回転を返す
	}
	return 0.0f; // プレイヤースポーンマーカーが見つからない場合は0を返す
}
