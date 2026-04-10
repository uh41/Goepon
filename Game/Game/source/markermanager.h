/*********************************************************************/
// * \file   markermanager.h
// * \brief  マーカー（スポーン位置など）管理クラス
// *
// * \author 鈴木裕稀
/*********************************************************************/

#pragma once
#include "appframe.h"

class MarkerManager
{
public:
	enum class MarkerType
	{
		PLAYER_SPAWN,	// プレイヤースポーン位置
		ENEMY_STATIC,	// 敵の静止位置
		ENEMY_MOVE,		// 敵の移動位置
		ENEMY_DOG,		// 敵の犬位置
		UNKNOWN,		// 不明なマーカータイプ
	};

	struct MarkerData
	{
		MarkerType type;	// マーカーの種類
		std::string objectName;// オブジェクト名
		std::string customId; // カスタムID
		nlohmann::json jsonData; // JSONデータ
		vec::Vec3 pos;	// マーカーの位置
		float rotation; // マーカーの回転
		MarkerData() : type(MarkerType::UNKNOWN), rotation(0.0f) {} // デフォルトコンストラクタ
	};

	static bool Initialize(); // 初期化
	static MarkerType GetMarkerType(const std::string& objectName); // オブジェクト名からマーカータイプを取得する関数
	static MarkerData ParseMarkerFromJson(const nlohmann::json& object); // JSONオブジェクトからマーカーデータを解析する関数

	static const MarkerData* GetMarkerData(const std::string& customId); // カスタムIDからマーカーデータを取得する関数
	static void RegisterMarkerData(const MarkerData& data); // マーカーデータを登録する関数
	static void ClearAllMarker(); // 全てのマーカーデータをクリアする関数
	static at::vec<MarkerData> GetMarkerType(MarkerType type); // マーカータイプからマーカーデータのリストを取得する関数
	static vec::Vec3 GetPlayerSpawnPosition(); // プレイヤースポーン位置を取得する関数
	static float GetPlayerSpawnRotation(); // プレイヤースポーンの回転を取得する関数

private:
	static at::umtc<std::string, MarkerData> _marker; // カスタムIDとマーカーのマップ
	static at::umtc<MarkerType, at::vec<MarkerData>> _markerType; // マーカータイプとマーカーデータのリストのマップ
};

