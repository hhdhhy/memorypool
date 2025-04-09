
#include "ThreadCache.h"
#include "CentralCache.h"
#include "PageCache.h"

ThreadCache &ThreadCache::getinstance()
{
    static thread_local ThreadCache  instance;
    return instance;
}

ThreadCache::~ThreadCache()
{
    for(int i=0;i<LIST_SIZE;++i)
    {
        if(free_list_[i]!=nullptr)
        CentralCache::getinstance().giveback_mem(free_list_[i],i);
    }
}

void *ThreadCache::allocate(std::size_t size)
{   

    if(size > MAX_BYTES)
    {
        return malloc(size);
    }
    size=round_up(size);
    std::size_t idx= get_idx(size);
    void *ptr=nullptr;
    if(free_list_[idx] == nullptr)
    {
        ptr=obtain(size,idx);
    }
    else
    {
        list_size_[idx]--;
        ptr = free_list_[idx];
        free_list_[idx] = next(ptr);
    }
    next(ptr)=nullptr;
    return ptr;
}

void ThreadCache::deallocate(void *ptr)
{
    Span* span  =PageCache::getinstance().ptr_to_span(ptr);
    // if(size>MAX_BYTES)
    if(span==nullptr)
    {
        free(ptr);
        return;
    }
    std::size_t size=span->mem_size_;
    size=round_up(size);
    std::size_t idx= get_idx(size);
    next(ptr) = free_list_[idx];
    free_list_[idx] = ptr;
    list_size_[idx]++;

    // if(list_size_[idx] >256)
    if(list_size_[idx] >max_size_[idx]*2)
    {
        giveback(size,idx);
    }
}

void* ThreadCache::obtain(std::size_t size,std::size_t idx)//批量获取
{
    if(max_size_[idx]==0)
    max_size_[idx]=2;
    std::size_t num=std::min(max_size_[idx],get_num(size));
    if(num==max_size_[idx])
    max_size_[idx]+=2;
    void* begin=nullptr; 
    std::size_t get_num=CentralCache::getinstance().get_mem(begin,size,idx,num);
    list_size_[idx]+=get_num-1;
    if(get_num>1)
    {
        free_list_[idx] = next(begin);//放的是ptr的下一个地址
    }
    return begin;
}
void ThreadCache::giveback(std::size_t size,std::size_t idx)
{
    
    // std::size_t ret_num=std::max(list_size_[idx]/4,static_cast<std::size_t>(1));
    std::size_t ret_num=std::min(list_size_[idx],std::max((std::size_t)64,max_size_[idx]));
    
    void *ptr=free_list_[idx];

    for(int i=0;i<(int)ret_num-1;++i)
    {
        ptr=next(ptr);
    }
    

    void *ret_ptr=free_list_[idx];
    free_list_[idx]=next(ptr);
 
    next(ptr)=nullptr;
    
    //返还给CentralCache
    std::size_t get= CentralCache::getinstance().giveback_mem(ret_ptr,idx);
    
    list_size_[idx]-=ret_num;
}
