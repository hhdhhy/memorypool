#pragma once
#include <array>
#include "Memorypool.h"
class ThreadCache
{

public:
    static ThreadCache& getinstance();
    ~ThreadCache();
    void* allocate(std::size_t size);
    void deallocate(void *ptr);
    // void deallocate(void *ptr, std::size_t size);

private:
    
    void* obtain(std::size_t size,std::size_t idx);
    void giveback(std::size_t size, std::size_t idx);

    void *free_list_[LIST_SIZE];
    std::size_t list_size_[LIST_SIZE];
    std::size_t max_size_[LIST_SIZE];//慢启动
    
};
