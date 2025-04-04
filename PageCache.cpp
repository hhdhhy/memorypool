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
            for(int i = 0;i < page_num;i++)
            span_map_[span->Pid_+i] = span;
            span->is_used_ = true;
            return span;
        }
        
        for(int i=idx+1;i<PAGE_MAX_NUM;++i)//找到大于page_num的span
        {
            if(!span_list_[i].empty())
            {
                Span* span = span_list_[i].get_head();
                Span* new_span =split(span,page_num);
                for(int j = 0;j < page_num;j++)
                span_map_[new_span->Pid_+j] = new_span;
                new_span->is_used_ = true;
                return new_span;
            }
        }

    }
    
    Span* span = system_alloc(PAGE_MAX_NUM);
    Span* new_span =split(span,page_num);
    
    //减小临界区
    {
        std::lock_guard<std::mutex> lock(mutex_);
        for(int j = 0;j < page_num;j++)
        span_map_[new_span->Pid_+j] = new_span;
        span_map_[span->Pid_]=span;
        span_map_[span->Pid_+page_num-1]=span;
        span_list_[span->num_-1].push_front(span);
        new_span->is_used_ = true;
    }
    
    return new_span;
}
void PageCache::giveback_span(Span *span)
{
    std::lock_guard<std::mutex> lock(mutex_);
    while(true)//向左
    {
        auto it = span_map_.find(span->Pid_-1);
        if(it==span_map_.end())
        {
            break;
        }
        Span* nspan=it->second;  
        if(nspan->is_used_==true)
        {
            break;
        }
        if(nspan->num_+span->num_>PAGE_MAX_NUM)
        {
            break;
        }
        span->num_ += nspan->num_;
        span->Pid_ =nspan->Pid_;
        //不用从map删掉不会被用到并且等以后添加的时候会顶替掉
        span_list_[nspan->num_].remove(nspan);
        delete nspan;
    }
    while(true)//向右
    {
        auto it = span_map_.find(span->Pid_+span->num_);
        if(it==span_map_.end())
        {
            break;
        }
        Span* nspan=it->second;  
        if(nspan->is_used_==true)
        {
            break;
        }
        if(nspan->num_+span->num_>PAGE_MAX_NUM)
        {
            break;
        }
        span->num_ += nspan->num_;
        //不用从map删掉不会被用到并且等以后添加的时候会顶替掉
        span_list_[nspan->num_].remove(nspan);
        delete nspan;
    }

    span_list_[span->num_].push_front(span);
    span->is_used_ = false;
    span_map_[span->Pid_]=span;
    span_map_[span->Pid_+span->num_-1]=span;
}
Span *split(Span *span, std::size_t page_num) // sanpan切下来page_num个page 返回切下来的的span
{
    Span* new_span = new Span;
    new_span->num_=page_num;
    span->num_-=page_num; 
    new_span->Pid_ = span->Pid_+span->num_;    
    return new_span;
}

Span* PageCache::system_alloc(std::size_t page_num)//获取page_num个page的span
{
    void* ptr=mmap(nullptr,page_num<<PAGE_SHIFT,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS,-1,0); 
    if(ptr==MAP_FAILED)
    {
        return nullptr;
    }
    Span *span = new Span;
    span->num_=page_num;
    span->Pid_=get_page_id(ptr);
    return span;
}

Span* PageCache::ptr_to_span(void *ptr)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto it= span_map_.find(get_page_id(ptr));
    if(it==span_map_.end())
    {
        abort();
    }
    return it->second;
}