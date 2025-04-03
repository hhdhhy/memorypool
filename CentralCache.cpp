#include "CentralCache.h"
#include "PageCache.h"

Span* CentralCache::get_span(std::size_t size,std::size_t idx)//获取一个可用span
{
    if(span_list_[idx].get_available_num())
    {
        Span *span = span_list_[idx].get_head();
        while(span)
        {
            if(span->num_)
            return span;
            span = span->next_;
        }
    }
    else
    {
        std::size_t page_num = get_page_num(idx*ALIGNMENT);

        Span* span= PageCache::getinstance().get_span(page_num);//从page cache中获取span
           
        //切分
        char* begin = reinterpret_cast<char*>(span->Pid_<<PAGE_SHIFT);
        char* end = begin + (span->num_<<PAGE_SHIFT);
        void *ptr= begin;
        while(begin!=end)
        {
            span->num_++;
            begin+=size;
            if(begin==end)
            {
                next(ptr)=nullptr;
                break;
            }
            next(ptr)=begin;
            ptr=next(ptr);
        }
        span_list_[idx].push_front(span);

        return span;
    }
} 
std::size_t CentralCache::fetch(void *&begin, void *&end, std::size_t size,std::size_t idx, size_t num)
{

    std::lock_guard<std::mutex> lock(span_list_[idx].get_mutex());

    Span* span = get_span(size,idx);

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
std::size_t CentralCache::get_page_num(std::size_t size)
{
    std::size_t num =0;//fix me

    std::size_t mem_size =num*size;

    size_t page_num = mem_size>>PAGE_SHIFT;
    if(!page_num)
    {
        page_num=1;
    }

    return page_num;
}