#include "CentralCache.h"
CentralCache::CentralCache(/* args */)
{
}

CentralCache& CentralCache::getinstance()
{
    static CentralCache instance;
    return instance;
}