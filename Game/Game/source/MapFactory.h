#pragma once
#include "MapBase.h"
#include"appframe.h"

enum class MapType
{
    Map1,
    Map2,
    Map3,
    // 今後追加されるマップタイプ
};

class MapFactory
{
public:
    static MapBase* CreateMap(MapType mapType);
    static MapType GetMapTypeFromString(const std::string& mapTypeName);
};