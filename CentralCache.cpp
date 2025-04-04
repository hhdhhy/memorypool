#include "CentralCache.h"
#include "PageCache.h"

Span* CentralCache::get_span(std::size_t size,std::size_t idx,std::unique_lock<std::mutex>&lock)//获取一个可用span
{
    if(span_list_[idx].available_num_)
    {
        Span *span = span_list_[idx].get_head();
        while(span)
        {
            if(span->num_)
            return span;
            span = span->next_;
        }
    }

    lock.unlock();

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

    lock.lock();
    span_list_[idx].push_front(span);
    span_list_[idx].available_num_++;

    return span;
    
} 
std::size_t CentralCache::get_mem(void *&begin, void *&end, std::size_t size,std::size_t idx, size_t num)
{
    std::unique_lock<std::mutex> lock(list_mutex_[idx]);
    Span* span = get_span(size,idx,lock);
    
    std::size_t get_num=std::min(num,span->num_);
    void *ptr=span->list_;

    for(int i=0;i<get_num-1;++i)
    ptr=next(ptr);

    begin=span->list_;
    end=ptr;
    span->list_=next(ptr);
    next(ptr)=nullptr;
    span->num_-=get_num;

    if(span->num_==0)
    {
        span_list_[idx].available_num_--;
    }
    return get_num;
}
void CentralCache::giveback_mem(void *ptr, std::size_t size, std::size_t idx)
{
    std::unique_lock<std::mutex> lock(list_mutex_[idx]);

    void* next_ptr=nullptr;
    while (ptr)
    {
        void* next_ptr=next(ptr);
        Span* span=PageCache::getinstance().ptr_to_span(ptr);
        next(ptr)=span->list_;
        if(span->list_)
        span->list_=ptr;
        span->available_num_++;
        if(span->available_num_==1)
        {
            span_list_[idx].available_num_++;
        }
        
        if(span->available_num_==span->num_)// 回收span 可以优化******* 
        {
            span_list_[idx].available_num_--;
            span_list_[idx].remove(span);
            span->list_=nullptr;//还回来是乱序的所以直接清空
            span->prev_=nullptr;
            span->next_=nullptr;
            lock.unlock();//减小临界区
            PageCache::getinstance().giveback_span(span);
            lock.lock();
        }
    }
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