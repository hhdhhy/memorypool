#include "PageCache.h"
#include <sys/mman.h>

PageCache::PageCache()
{
}



PageCache &PageCache::getinstance()
{
    static PageCache instance;
    return instance;
}

Span *PageCache::get_span(std::size_t page_num)
{
    std::size_t idx = page_num - 1;

    {
        std::lock_guard<std::mutex> lock(mutex_);

        if(!span_list_[idx].empty())//存在空闲span
        {
            Span* span = span_list_[idx].get_head();
            span_list_[idx].remove(span);
            span_
            return span;
        }
        
        for(int i=idx+1;i<PAGE_MAX_NUM;++i)//找到大于page_num的span
        {
            if(!span_list_[i].empty())
            {
                Span* span = span_list_[i].get_head();
                Span* new_span =split(span,page_num);
                return new_span;
            }
        }

    }
    
    Span* span = system_alloc(PAGE_MAX_NUM);
    Span* new_span =split(span,page_num);
    //减小临界区
    {
        std::lock_guard<std::mutex> lock(mutex_);
        span_list_[span->num_-1].push_front(span);
    }
    
    return new_span;
}
Span* split(Span* span,std::size_t page_num)//sanpan切下来page_num个page 返回切下来的的span
{
    Span* new_span = new Span;
    new_span->num_=page_num;
    span->num_-=page_num; 
    new_span->Pid_ = span->Pid_+span->num_;    
    return new_span;
}

Span* PageCache::system_alloc(std::size_t page_num)//获取page_num个page的span
{
    void* ptr=mmap(nullptr,page_num<<PAGE_SHIFT, 
    PROT_READ | PROT_WRITE,MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if(ptr==MAP_FAILED)
    {
        return nullptr;
    }
    Span *span = new Span;
    span->num_=page_num;
    span->Pid_=reinterpret_cast<std::size_t>(ptr)>>PAGE_SHIFT;
    return span;
}