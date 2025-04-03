#include "PageCache.h"

PageCache::PageCache()
{
}



PageCache &PageCache::getinstance()
{
    static PageCache instance;
    return instance;
}
