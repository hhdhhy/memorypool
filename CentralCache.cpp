#include "CentralCache.h"
#include "PageCache.h"

CentralCache* CentralCache:: instance_=new CentralCache;
CentralCache &CentralCache::getinstance()
{
    return *instance_;
}


Span *CentralCache::get_span(std::size_t size, std::size_t idx, std::unique_lock<std::mutex> &lock) // 获取一个可用span
{
    if(span_list_[idx].available_num_)
    {
        Span *span = span_list_[idx].get_head();
        while(span)
        {
            if(span->available_num_)
            return span;
            span = span->next_;
        }
    }

    lock.unlock();

    std::size_t page_num = get_page_num(size);

    Span* span= PageCache::getinstance().get_span(page_num);//从page cache中获取span
    

    char* begin = (char*)(span->Pid_ << PAGE_SHIFT);
	char* end = (char*)(begin + (span->page_num_ << PAGE_SHIFT));
	// 开始切分span管理的空间
    span->mem_size_=size;
	span->list_ = begin;// 管理的空间放到span->_freeList中

	void* ptr = begin; 

    span->num_=(span->page_num_ << PAGE_SHIFT)/size;// 计算span中可以管理的内存块数量
    span->available_num_=span->num_;

	for(int i=0;i<span->num_-1;++i)
    {
        begin+=size;    
        next(ptr)=begin;
        ptr=begin;
    }
    next(ptr)=nullptr;

    lock.lock();
    span_list_[idx].push_front(span);
    span_list_[idx].available_num_++;

    return span;
    
} 
std::size_t CentralCache::get_mem(void *&begin, std::size_t size,std::size_t idx, size_t num)
{
    std::unique_lock<std::mutex> lock(list_mutex_[idx]);
    Span* span = get_span(size,idx,lock);
    
    std::size_t get_num=std::min(span->available_num_,num);
    void *ptr=span->list_;
    assert(span); 
	assert(span->list_);
    for(int i=0;i<(int)get_num-1;++i)
    {
        ptr=next(ptr);
    }
    
    begin=span->list_;
    span->list_=next(ptr);
    next(ptr)=nullptr;
    span->available_num_-=get_num;

    if(span->available_num_==0)
    {
        span_list_[idx].available_num_--;
    }
    return get_num;
}
std::size_t CentralCache::giveback_mem(void *ptr, std::size_t idx)
{
    std::unique_lock<std::mutex> lock(list_mutex_[idx]);

    void* next_ptr=nullptr;
    int num=0;
    while (ptr)
    {
        num++;
        next_ptr=next(ptr);
        Span* span=PageCache::getinstance().ptr_to_span(ptr);
        next(ptr)=span->list_;
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
        ptr=next_ptr;
    }
    return num;
}

