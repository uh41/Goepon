/*********************************************************************/
// * \file   applicationblocal.h
// * \brief  アプリケーショングローバルクラス
// *
// * \author 鈴木裕稀
/*********************************************************************/

#pragma once
#include "appframe.h"

namespace soundserver
{
	class SoundServer;
}

class ApplicationGlobal
{
public:

	struct MapData
	{
		at::mst<int> modelHandle; // モデル名とハンドルのマップ
		at::vet<mymath::BLOCKPOS> blockPos; // ブロックの位置データ
	};

	struct StageObjectData
	{
		std::string objectName; // オブジェクト名
		nlohmann::json json;
	};

	struct PatrolPointInfo
	{
		vec::Vec3 pos{}; // パトロールポイントの位置
		int id = 0; // パトロールポイントのID
		float waitTime = 0.0f; // 待機時間
	};

	struct StageData
	{
		at::vet<StageObjectData> object; // ステージオブジェクトのデータ
		at::umtt < std::string, at::vet<vec::Vec3>> patrolGroup; // パトロールグループのデータ
		at::umtc<std::string, at::vec<PatrolPointInfo>> patrolPointInfo; // パトロールポイントの情報
	};

	ApplicationGlobal();
	virtual ~ApplicationGlobal();

	bool Init();    // 初期化

	bool UnloadMapData(const std::string& mapName); // マップデータをアンロードする関数
	void UnloadAllMapData();

	bool UnloadStageData(const std::string& stageName);
	void UnloadAllStageData();

	int _iCgCursor;       // カーソル画像ハンドル
	at::spc<soundserver::SoundServer> _soundServer;

	bool IsLoading() const { return _isLoading; } // ローディング中かどうか
	int GetLoadProgress() const { return _loadProgress; } // ロード進行度を取得
	void UpdateLoadProgress(); // ロード進行度を更新（例: ロードが完了したら100に設定）

	bool LoadMapData(const std::string& mapName, const std::string& jsonFileName, const std::string& jsonObjectName = "stage"); // マップデータをJSONから読み込む関数

	const MapData* GetMapData(const std::string& mapName); // マップデータを取得

	bool LoadStageData(const std::string& stageName, const std::string& jsonFileName, const std::string& jsonObjectName = "stage");
	const StageData* GetStageData(const std::string& stageName); // ステージデータを取得
	
	bool EnsureStageDataLoad(const std::string& stageId); // ステージデータがロードされていない場合にロードする関数

	// ステージ管理機能
	const at::vec<std::string>& GetStageList() const { return _stageList; } // ステージ名のリストを取得
	std::string GetNextStageId(const std::string& currentStageId) const;       // 次のステージ名を取得
	bool HasNextStage(const std::string& currentStageId) const;                // 次のステージが存在するかどうか

protected:
	bool _isLoading; // ローディング中フラグ
	int _loadProgress; // ロード進行度（0-100）

	at::mst<MapData> _mapData; // マップ名とマップデータのマップ
	at::mst<StageData> _stageData; // ステージ名とステージデータのマップ

	at::vec<std::string> _stageList; // ステージ名のリスト
};

// 他のソースでgGlobalを使えるように
extern ApplicationGlobal gGlobal;
