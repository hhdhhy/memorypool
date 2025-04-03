#include "CentralCache.h"


Span* CentralCache::get_span(size_t idx)//获取一个可用span
{


} 
std::size_t CentralCache::fetch(void *&begin, void *&end, std::size_t idx, size_t num)
{
    std::lock_guard<std::mutex> lock(span_list_[idx].get_mutex());

    Span* span = get_span(idx);

    std::size_t get_num=std::min(num,span->num_);
    void *ptr=span->list_;

    for(int i=0;i<get_num-1;++i)
    ptr=next(ptr);

    begin=span->list_;
    end=ptr;
    span->list_=next(ptr);
    next(ptr)=nullptr;
    span->num_-=get_num;

    return get_num;
}
CentralCache::CentralCache()
{
    
}

CentralCache& CentralCache::getinstance()
{
    static CentralCache instance;
    return instance;
}