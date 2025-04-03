#pragma once
#include <array>
#include "Memorypool.h"
class ThreadCache
{
    static const std::size_t LIST_MAX_SIZE = 128;
    static const size_t MAX_BATCH_SIZE = 4 * 1024; // 4KB
public:
    static ThreadCache& getinstance();

    void* allocate(std::size_t size);
    void deallocate(void* ptr, std::size_t size);

private:
    ThreadCache();
    void* obtain(std::size_t idx);
    void giveback(std::size_t idx);

    std::array<void *,FREE_LIST_SIZE> free_list_;
    std::array<std::size_t,FREE_LIST_SIZE> list_size_;
};
