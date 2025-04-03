
#include "ThreadCache.h"
#include "CentralCache.h"

ThreadCache &ThreadCache::getinstance()
{
    static thread_local ThreadCache  instance;
    return instance;
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
    return ptr;
}

void ThreadCache::deallocate(void *ptr, std::size_t size)
{
    if(size>ALIGNMENT)
    {
        free(ptr);
        return;
    }
    std::size_t idx= get_idx(size);
    next(ptr) = free_list_[idx];
    free_list_[idx] = ptr;
    list_size_[idx]++;

    if(list_size_[idx] > LIST_MAX_SIZE)
    {
        giveback(idx);
    }
}

void* ThreadCache::obtain(std::size_t size,std::size_t idx)//批量获取
{

    std::size_t num=std::max(MAX_BATCH_SIZE/size,static_cast<std::size_t>(1));

    void* begin=nullptr; 
    void* end=nullptr;
    std::size_t get_num=CentralCache::getinstance().fetch(begin,end,size,idx,num);
    list_size_[idx]+=get_num-1;
    if(get_num>1)
    {
        free_list_[idx] = next(begin);//放的是ptr的下一个地址
    }
    return begin;
}
void ThreadCache::giveback(std::size_t idx)
{
    std::size_t size=idx*ALIGNMENT;
    
    std::size_t save_num=std::max(list_size_[idx]/4,static_cast<std::size_t>(1));

    void *ptr=free_list_[idx];

    for(int i=0;i<save_num-1;++i)
    ptr=next(ptr);

    void *next_ptr=next(ptr);
    next(ptr)=nullptr;
    
    // 没写完
    //返还给CentralCache

    list_size_[idx]=save_num;
}
