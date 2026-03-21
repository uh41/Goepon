/*********************************************************************/
// * \file   applicationblocal.cpp
// * \brief  アプリケーショングローバルクラス
// *
// * \author 鈴木裕稀
// * \date   2025/12/15
// * \作業内容: 新規作成 鈴木裕稀　2025/12/15
/*********************************************************************/

#include "applicationglobal.h"
#include "appframe.h"


// 実体
ApplicationGlobal gGlobal;


// コンストラクタ
ApplicationGlobal::ApplicationGlobal()
{
	_iCgCursor = -1;
	_isLoading = false;
	_loadProgress = 0;

	_stageList = 
	{
		"Stage1",
		"Stage2",
		"Stage3"
	};
}

// デストラクタ
ApplicationGlobal::~ApplicationGlobal()
{
	for(auto& mapPair : _mapData)
	{
		for(auto& modelPair : mapPair.second.modelHandle)
		{
			if(modelPair.second >= 0)
			{
				MV1DeleteModel(modelPair.second);
			}
		}
		mapPair.second.modelHandle.clear();
		mapPair.second.blockPos.clear();
	}
	_mapData.clear();

	// ステージデータのクリア
	_stageData.clear();
	//ResourceServer::Release();
	// 何もしない
}

bool ApplicationGlobal::LoadMapData(const std::string& mapName, const std::string& jsonFileName, const std::string& jsonObjectName)
{
	std::string path = "res/map/";

	std::ifstream ifs(path + jsonFileName);
	if (!ifs.is_open())
	{
		return false; // ファイルが開けない場合はfalseを返す
	}

	nlohmann::json json;

	ifs >> json;
	ifs.close();

	MapData map;

	nlohmann::json stage = json.at(jsonObjectName);
	for(auto&& data : stage)
	{
		mymath::BLOCKPOS pos;


		// 明示的に初期化しておく
		pos.name.clear();
		pos.x = pos.y = pos.z = 0.0f;
		pos.rx = pos.ry = pos.rz = 0.0f;
		pos.sx = pos.sy = pos.sz = 1.0f;
		pos.modelHandle = -1;
		pos.drawFrame = -1;
		pos.collisionFrame = -1;

		data.at("objectName").get_to(pos.name);
		// UEは左手座標系/Zup →左手座標系/Yup に変換しつつ取得
		data.at("translate").at("x").get_to(pos.x);
		data.at("translate").at("z").get_to(pos.y);
		data.at("translate").at("y").get_to(pos.z);
		pos.z *= -1.0f;
		data.at("rotate").at("x").get_to(pos.rx);
		data.at("rotate").at("z").get_to(pos.ry);
		data.at("rotate").at("y").get_to(pos.rz);
		pos.rx = DEG2RAD(pos.rx);
		pos.ry = DEG2RAD(pos.ry);
		pos.rz = DEG2RAD(pos.rz);
		data.at("scale").at("x").get_to(pos.sx);
		data.at("scale").at("z").get_to(pos.sy);
		data.at("scale").at("y").get_to(pos.sz);

		// 名前のモデルがすでに読み込み済か？
		if(map.modelHandle.count(pos.name) == 0)
		{
			// まだ読み込まれていない。読み込みを行う
			std::string filename = path + pos.name + ".mv1";
			int h = MV1LoadModel(filename.c_str());
			map.modelHandle[pos.name] = h;
		}

		// 名前から使うモデルハンドル＆表示フレームを決める
		if(map.modelHandle.count(pos.name) > 0)
		{
			pos.modelHandle = map.modelHandle[pos.name];
			if(pos.modelHandle >= 0)
			{
				pos.drawFrame = MV1SearchFrame(pos.modelHandle, pos.name.c_str());
			}
		}

		// Collision フレームは各ブロックごとに求めて保存
		pos.collisionFrame = -1;
		if(pos.modelHandle >= 0)
		{
			// まずは想定名で検索
			pos.collisionFrame = MV1SearchFrame(pos.modelHandle, "collision");

			// 見つからなければモデル内フレームを列挙して "Collision" を含むものを探す
			if(pos.collisionFrame < 0)
			{
				int frameNum = MV1GetFrameNum(pos.modelHandle);
				for(int fi = 0; fi < frameNum; ++fi)
				{
					const char* fname = MV1GetFrameName(pos.modelHandle, fi);
					if(fname)
					{
						std::string s(fname);
						if(s.find("Collision") != std::string::npos || s.find("collision") != std::string::npos)
						{
							pos.collisionFrame = fi;
							break;
						}
					}
				}
			}

			// 見つかったらコリジョン情報を生成
			if(pos.collisionFrame >= 0)
			{
				MV1SetupCollInfo(pos.modelHandle, pos.collisionFrame, 16, 16, 16);
				MV1SetFrameVisible(pos.modelHandle, pos.collisionFrame, FALSE);
			}
		}

		// データをコンテナに追加（モデル番号があれば）
		if(pos.modelHandle != -1)
		{
			map.blockPos.push_back(pos);
		}
	}


	// マップデータを保存
	_mapData[mapName] = map;

	return true; // 成功した場合はtrueを返す
}

bool ApplicationGlobal::LoadStageData(const std::string& stageName, const std::string& jsonFileName, const std::string& jsonObjectName)
{
	std::string path = "res/map/";

	std::ifstream ifs(path + jsonFileName);
	if(!ifs.is_open())
	{
		return false;
	}

	nlohmann::json jsonData;
	ifs >> jsonData;
	ifs.close();

	nlohmann::json stage = jsonData.at(jsonObjectName);

	StageData stageData;

	// 全オブジェクトを保存
	for(auto& object : stage)
	{
		StageObjectData objData;
		objData.objectName = object.at("objectName");
		objData.json = object;

		// S_MarkerR（巡回点）をグループ化
		if(objData.objectName == "S_MarkerR")
		{
			vec::Vec3 pos;
			object.at("translate").at("x").get_to(pos.x);
			object.at("translate").at("y").get_to(pos.z); // UE:y -> DXLib:z
			object.at("translate").at("z").get_to(pos.y); // UE:z -> DXLib:y
			pos.z *= -1.0f;

			std::string gid = "";
			if(object.contains("customId"))
			{
				object.at("customId").get_to(gid);
			}
			stageData.patrolGroup[gid].push_back(pos);

			PatrolPointInfo info{};
			info.pos = pos;

			if(object.contains("direction"))
			{
				auto& dir = object.at("direction");
				if(dir.is_number_integer())
				{
					info.id = dir.get<int>();
				}
				else if(dir.is_string())
				{
					std::string dirStr = dir.get<std::string>();
					if(!dirStr.empty())
					{
						info.id = std::stoi(dirStr);
					}
				}

				if(object.contains("waittime"))
				{
					auto& waitTime = object.at("waittime");
					if(waitTime.is_number())
					{
						info.waitTime = waitTime.get<float>();
					}
					else if(waitTime.is_string())
					{
						std::string waitTimeStr = waitTime.get<std::string>();
						if(!waitTimeStr.empty())
						{
							info.waitTime = std::stof(waitTimeStr);
						}
					}
				}
			}
			stageData.patrolPointInfo[gid].push_back(info);
		}

		stageData.object.push_back(objData);
	}

	// ステージデータを保存
	_stageData[stageName] = stageData;
	return true;
}

const ApplicationGlobal::MapData* ApplicationGlobal::GetMapData(const std::string& mapName)
{
	auto it = _mapData.find(mapName);
	if(it != _mapData.end())
	{
		return &(it->second);
	}
	return nullptr; // 見つからない場合はnullptrを返す
}

const ApplicationGlobal::StageData* ApplicationGlobal::GetStageData(const std::string& stageName)
{
	auto it = _stageData.find(stageName);
	if(it != _stageData.end())
	{
		return &(it->second);
	}
	return nullptr; // 見つからない場合はnullptrを返す
}

// 初期化
bool ApplicationGlobal::Init()
{
	_isLoading = true;
	_loadProgress = 0;


	_iCgCursor = LoadGraph("res/cursor.png");

	// マップ用
	LoadMapData("Map1", "stage1_0319x.json", "stage");	// 1ステージ目
	LoadMapData("Map2", "stage2_0311.json", "stage");	// 2ステージ目
	LoadMapData("Map3", "stage3_0312.json", "stage");	// 3ステージ目

	//マーカー用
	LoadStageData("Stage1", "stage1_0319x.json", "stage"); // 1ステージ目
	LoadStageData("Stage2", "stage2_0319.json", "stage");     // 2ステージ目
	LoadStageData("Stage3", "stage3_0312.json", "stage");     // 3ステージ目

	SetUseASyncLoadFlag(TRUE);

	// キャラクター関連モデル読み込み
	ResourceServer::MV1LoadModel(mv1::sigaraki);
	ResourceServer::MV1LoadModel(mv1::SK_Dog);
	ResourceServer::MV1LoadModel(mv1::SK_busi_multimotion);
	ResourceServer::MV1LoadModel(mv1::GameClear_Tanuki);
	ResourceServer::MV1LoadModel(mv1::SK_multimotionTanuHuman_06);
	ResourceServer::MV1LoadModel(mv1::gamestart);

								 	 
	// マップ関連モデル読み込み	  	 
	ResourceServer::MV1LoadModel(mv1::skysphere);
	ResourceServer::MV1LoadModel(mv1::SM_stage1_0301);
	ResourceServer::MV1LoadModel(mv1::SM_stage20303);
	ResourceServer::MV1LoadModel(mv1::SM_stage3);
	ResourceServer::MV1LoadModel(mv1::S_Marker_Event);

	// その他
	ResourceServer::MV1LoadModel(mv1::tuzura_05);
	ResourceServer::MV1LoadModel(mv1::Goal);
	ResourceServer::LoadGraph(texture::makimono);
	ResourceServer::LoadGraph(texture::TX_Tresure5);
	ResourceServer::LoadGraph(texture::TX_ase);
	ResourceServer::LoadGraph(texture::TX_change);
	ResourceServer::LoadGraph(texture::TX_doyax);
	ResourceServer::LoadGraph(texture::TX_find);
	ResourceServer::LoadGraph(texture::TX_hatena);
	ResourceServer::LoadGraph(texture::TX_nakix);
	ResourceServer::LoadGraph(texture::TX_walk);
	ResourceServer::LoadGraph(texture::TX_shirimochi);
	ResourceServer::LoadGraph(texture::TX_kizetsu);
	ResourceServer::LoadGraph(img::UI_Makimono);
	ResourceServer::LoadGraph(img::UI_Tanubito);
	ResourceServer::LoadGraph(img::UI_Tanumono);

	if(!_soundServer)
	{
		_soundServer = std::make_shared<soundserver::SoundServer>();
	}
	_soundServer->Add("bgminitialize", std::make_shared<soundserver::SoundItemBGM>(mp3::BGM_stage_normal));
	_soundServer->Add("bgmChenge", std::make_shared<soundserver::SoundItemBGM>(mp3::BGM_enemy_found));
	_soundServer->Add("1", std::make_shared<soundserver::SoundItemSE>(mp3::tanuki_walk, soundserver::SoundItemBase::FLG_LOOP));
	_soundServer->Add("2", std::make_shared<soundserver::SoundItemSE>(mp3::tanuki_henshin_start));
	_soundServer->Add("35", std::make_shared<soundserver::SoundItemSE>(mp3::bush_found));
	_soundServer->Add("31", std::make_shared<soundserver::SoundItemSE>(mp3::bushi_walk, soundserver::SoundItemBase::FLG_3D));
	_soundServer->Add("4", std::make_shared<soundserver::SoundItemSE>(mp3::tanuki_Tresure_open));
	_soundServer->Add("3", std::make_shared<soundserver::SoundItemSE>(mp3::tanuki_henshinfinish));
	_soundServer->Add("10", std::make_shared<soundserver::SoundItemSE>(mp3::tanubito_tailattack));
	_soundServer->Add("30", std::make_shared<soundserver::SoundItemSE>(mp3::bushi_stun_start));
	_soundServer->Add("40", std::make_shared<soundserver::SoundItemSE>(mp3::dog_howling,soundserver::SoundItemBase::FLG_3D | soundserver::SoundItemBase::FLG_LOOP));
	_soundServer->Add("11", std::make_shared<soundserver::SoundItemSE>(mp3::tanubito_walk, soundserver::SoundItemBase::FLG_LOOP));
	_soundServer->Add("50", std::make_shared<soundserver::SoundItemSE>(mp3::okimono_move));
	_soundServer->Add("60", std::make_shared<soundserver::SoundItemSE>(mp3::UI_charin, soundserver::SoundItemBase::FLG_LOOP));
	_soundServer->Add("33", std::make_shared<soundserver::SoundItemVoice>(mp3::bushi_sight_off, soundserver::SoundItemBase::FLG_3D));
	_soundServer->Add("32", std::make_shared<soundserver::SoundItemVoice>(mp3::bushi_tail_hit, soundserver::SoundItemBase::FLG_3D));
	_soundServer->Add("34", std::make_shared<soundserver::SoundItemVoice>(mp3::bushi_stunfinish, soundserver::SoundItemBase::FLG_3D));
	_soundServer->Add("61", std::make_shared<soundserver::SoundItemSE>(mp3::UI_no_makimono));
	_soundServer->Add("120", std::make_shared<soundserver::SoundItemBGM>(mp3::Ending));
	_soundServer->Add("110", std::make_shared<soundserver::SoundItemBGM>(mp3::Prologe));
	_soundServer->Add("5", std::make_shared<soundserver::SoundItemSE>(mp3::tanuki_run));
	_soundServer->Add("41", std::make_shared<soundserver::SoundItemSE>(mp3::Dog_walk, soundserver::SoundItemBase::FLG_3D));
	_soundServer->Add("63", std::make_shared<soundserver::SoundItemSE>(mp3::UI_Henshin_pon));
	_soundServer->Add("voice1", std::make_shared<soundserver::SoundItemVoice>(mp3::voice1, soundserver::SoundItemBase::FLG_3D));
	_soundServer->Add("voice2", std::make_shared<soundserver::SoundItemVoice>(mp3::voice2, soundserver::SoundItemBase::FLG_3D));
	_soundServer->Add("voice3", std::make_shared<soundserver::SoundItemVoice>(mp3::voice3, soundserver::SoundItemBase::FLG_3D));


	return true;
}

void ApplicationGlobal::UpdateLoadProgress()
{
	if(_isLoading)
	{
		if(GetASyncLoadNum() == 0)
		{
			_loadProgress = 100;
			_isLoading = false;
			SetUseASyncLoadFlag(FALSE);
		}
		else
		{
			int maxLoadNum = 0;
			if(GetASyncLoadNum() > maxLoadNum)
			{
				maxLoadNum = GetASyncLoadNum();
			}

			if(maxLoadNum > 0)
			{
				_loadProgress = (int)(((float)(maxLoadNum - GetASyncLoadNum()) / (float)maxLoadNum) * 100.0f);
			}
		}
	}
}

std::string ApplicationGlobal::GetNextStageId(const std::string& currentStageId) const
{
	if(_stageList.empty())
	{
		return {};
	}

	for(size_t i = 0; i < _stageList.size(); ++i)
	{
		if(_stageList[i] == currentStageId)
		{
			const size_t next = i + 1;
			if(next < _stageList.size())
			{
				return _stageList[next];
			}
			return {}; // 次がなければ空文字
		}
	}

	return {}; // 見つからなければ空文字
}

// 次のステージが存在するかどうか
bool ApplicationGlobal::HasNextStage(const std::string& currentStageId) const
{
	return !GetNextStageId(currentStageId).empty();
}