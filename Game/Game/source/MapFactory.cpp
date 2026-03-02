#ifdef _DEBUG
#include <crtdbg.h>
#define NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
#else
#define NEW new
#endif

#include "MapFactory.h"
#include "Map1.h"
#include "Map2.h" // 新しいマップクラス

MapBase* MapFactory::CreateMap(MapType mapType)
{
    switch(mapType)
    {
        case MapType::Map1:
            return NEW Map1();
        case MapType::Map2:
            return NEW Map2();
        default:
            return nullptr;
    }
}

MapType MapFactory::GetMapTypeFromString(const std::string& mapTypeName)
{
    if(mapTypeName == "Map1") return MapType::Map1;
    if(mapTypeName == "Map2") return MapType::Map2;

    // デフォルトはMap1
    return MapType::Map1;
}