/*********************************************************************/
// * \file   ObjectServer.cpp
// * \brief  オブジェクトサーバークラス
// *
// * \author 石森虹大
// * \date   2026/1/5
// * \作業内容: 新規作成 石森虹大　2026/1/5
/*********************************************************************/


#include"ObjectServer.h"
#include"PlayerBase.h"
#include"enemy.h"
#include"ModeGame.h"
#include<algorithm>
#include<fstream>

namespace
{
	// 文字列の前後の空白を削除してコピーを返す
	static std::string TrimCopy(std::string s)
	{
		auto notSpace = [](unsigned char c) { return !std::isspace(c); };
		s.erase(s.begin(), std::find_if(s.begin(), s.end(), notSpace));
		s.erase(std::find_if(s.rbegin(), s.rend(), notSpace).base(), s.end());
		return s;
	}
}


ObjectServer::ObjectServer(ModeGame* game)
:_game(game)
{
	Intialize();
}

ObjectServer::~ObjectServer()
{
	Terminate();
}

bool ObjectServer::Intialize()
{
	_factory.clear();
	_factory.emplace("PlayerTanuki", []() { return new PlayerTanuki(); });
	_factory.emplace("Player      ", []() { return new Player();       });
	_factory.emplace("Enemy       ", []() { return new Enemy();        });
	_factory.emplace("Treasure    ", []() { return new Treasure();     });
	_factory.emplace("Map         ", []() { return new Map();          });
	return true;
}

bool ObjectServer::Terminate()
{
	//管理しているオブジェクトをすべて削除
	ClearObject();
	return true;
}

bool ObjectServer::Process()
{
	// オブジェクトを巡回処理
	for(auto iter = _objects.begin(); iter != _objects.end(); ++iter)
	{
		if((*iter)->Process())
		{

		}
		else
		{
			return false;
		}
	}
	return true;
}

bool ObjectServer::Render()
{
	// オブジェクトを巡回描画
	for(int i = 0; i < _objects.size(); ++i)
	{
		if(!_objects[i]->Render())
		{
			return false;
		}
	}
	return true;
}

// オブジェクト追加
void ObjectServer::AddObject(ObjectBase* obj)
{
	// 既に追加されているか
	auto iter = std::find(_deleteObj.begin(), _deleteObj.end(), obj);
	if(iter != _deleteObj.end())
	{
		return;
	}
	// まだ、追加されていないだけで予約は入っているのでは？
	iter = std::find(_addObj.begin(), _addObj.end(), obj);
	if(iter != _addObj.end())
	{
		return;
	}

	// 初期化
	obj->Initialize();
	_addObj.emplace_back(obj);
}

// オブジェクト削除
void ObjectServer::DeleteObject(ObjectBase* obj)
{
	//既に削除予約されているか
	auto iter = std::find(_deleteObj.begin(), _deleteObj.end(), obj);
	if (iter != _deleteObj.end()) {
		return;
	}

	iter = std::find(_addObj.begin(), _addObj.end(), obj);
	if (iter != _addObj.end()) {
		(*iter)->Terminate();
		delete (*iter);
		_addObj.erase(iter);
		return;
	}
	_deleteObj.emplace_back(obj);
}

//_objects _add _delete のコンテナが保持するアドレスをdelete、各コンテナのサイズを０にする
bool ObjectServer::ClearObject()
{
	for (auto&& obj : _objects) 
	{
		delete obj;
	}
	_objects.clear();

	for (auto&& obj : _addObj) 
	{
		obj->Terminate();
		delete obj;
	}
	_addObj.clear();

	_deleteObj.clear();
	return true;
}

// オブジェクト巡回処理の前に呼び出す初期化
bool ObjectServer::ProcessInit()
{
	// 巡回処理をする前にオブジェクトの追加と削除をしておく
	for(auto && addObj : _addObj)
	{
		//実際に追加されてから、初期化する
		addObj->Initialize();
		_objects.emplace_back(addObj);
	}
	_addObj.clear();

	// 削除処理
	for (auto&& deleteObj : _deleteObj)
	{
		auto iter = std::find(_objects.begin(), _objects.end(), deleteObj);
		if (iter != _objects.end()) 
		{
			(*iter)->Terminate();
			delete (*iter);
			_objects.erase(iter);
		}
	}
	_deleteObj.clear();

	return true;
}

// オブジェクト生成関数
ObjectBase* ObjectServer::CreateByType(const std::string& type)const
{
	// ファクトリマップから生成関数を取得
	auto it = _factory.find(type);
	// 見つからなかった場合はnullptrを返す
	if(it == _factory.end())
	{
		return nullptr;
	}

	// 生成関数を呼び出してオブジェクトを生成・返却
	return it->second();
}

// Jsonからオブジェクト生成
bool ObjectServer::SpawnFromJson(const nlohmann::json& objectJson)
{
	std::string type = objectJson.value("type", "");
	type = TrimCopy(type);

	// type が無ければ失敗
	if(type.empty())
	{
		return false;
	}

	// オブジェクト生成
	ObjectBase* obj = CreateByType(type);
	if(obj == nullptr)
	{
		return false;
	}

	// Jsonデータをセット
	obj->SetJsonDataUE(objectJson);

	// オブジェクト追加
	AddObject(obj);

	// 特定クラスの登録
	if(auto ch = dynamic_cast<CharaBase*>(obj))
	{
		_charas.emplace_back(ch);
	}
	// プレイヤー登録
	if(auto player = dynamic_cast<Player*>(obj))
	{
		_player = player;
	}

	return true;
}

// カテゴリからオブジェクト生成
bool ObjectServer::SpawnFromCategory(const nlohmann::json& root, const char* category, const char* type)
{
	// カテゴリが無ければスキップ
	if(!root.contains(category))
	{
		return true; // 無いカテゴリはスキップ
	}

	// 各オブジェクトを生成
	for(const auto& obj : root.at(category))
	{
		nlohmann::json patched = obj;// コピー
		patched["type"] = type;      // type を補完

		// オブジェクト生成
		if(!SpawnFromJson(patched))
		{
			return false;
		}
	}
	return true;
}

// オブジェクトのレイアウトを読み込み、オブジェクトを生成
bool ObjectServer::LoadDate(const std::string& layoutJsonPath)
{
	std::ifstream layoutFile(layoutJsonPath);
	if(!layoutFile)
	{
		return false;
	}

	nlohmann::json layoutJson;
	layoutFile >> layoutJson;

	// 1) objects 形式があれば最優先（type を json 側で持てるので拡張が楽）
	if(layoutJson.contains("objects"))
	{
		for(const auto& obj : layoutJson.at("objects"))
		{
			if(!SpawnFromJson(obj))
			{
				return false;
			}
		}
		return true;
	}

	// 2) カテゴリ形式（既存資産）を type 補完して生成
	if(!SpawnFromCategory(layoutJson, "map", "Map")) return false;
	if(!SpawnFromCategory(layoutJson, "player", "Player")) return false;
	if(!SpawnFromCategory(layoutJson, "playerTanuki", "PlayerTanuki")) return false;
	if(!SpawnFromCategory(layoutJson, "enemy", "Enemy")) return false;
	if(!SpawnFromCategory(layoutJson, "treasure", "Treasure")) return false;

	return true;
}

