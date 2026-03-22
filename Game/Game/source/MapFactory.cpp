#ifdef _DEBUG
#include <crtdbg.h>
#define NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
#else
#define NEW new
#endif

#include "MapFactory.h"
#include "Map1.h"
#include "Map2.h" 
#include "Map3.h"

// マップタイプに応じたMapBaseのインスタンスを生成する関数
MapBase* MapFactory::CreateMap(MapType mapType)
{
    switch(mapType)
    {
        case MapType::Map1:
            return NEW Map1();
        case MapType::Map2:
            return NEW Map2();
		case MapType::Map3:
            return NEW Map3();
        default:
            return nullptr;
    }
}

MapBase* MapFactory::DestroyMap(MapBase* map)
{
    if(!map) return nullptr;
    map->Terminate();
    delete map;
    return nullptr;
}

// 文字列からMapTypeを取得する関数
MapType MapFactory::GetMapTypeFromString(const std::string& mapTypeName)
{
    if(mapTypeName == "Map1") return MapType::Map1;
    if(mapTypeName == "Map2") return MapType::Map2;
	if(mapTypeName == "Map3") return MapType::Map3;

    // デフォルトはMap1
    return MapType::Map1;
}