#pragma once
#include <algorithm>


constexpr std::size_t ALIGNMENT = sizeof (char*);// ALIGNMENT等于指针char*的大小
constexpr std::size_t MAX_BYTES = 256 * 1024; // 256KB
constexpr std::size_t FREE_LIST_SIZE = MAX_BYTES / ALIGNMENT; 

std::size_t get_idx(std::size_t size)
{
    size= std::max(size, ALIGNMENT);
    return (size + ALIGNMENT - 1) / ALIGNMENT - 1;//向上取整

}
size_t roundUp(size_t size)
{
    return (size + ALIGNMENT - 1) & ~(ALIGNMENT - 1);
}

inline void*& next(void* ptr)
{
    return *reinterpret_cast<void**>(ptr);
}



