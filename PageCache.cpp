#include "PageCache.h"

PageCache::PageCache()
{
}



PageCache &PageCache::getinstance()
{
    static PageCache instance;
    return instance;
}

Span *PageCache::getspan(std::size_t page_num)
{
   
    

}

