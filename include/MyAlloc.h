#include "ThreadCache.h"
void *MyAlloc(std::size_t size)
{
    return ThreadCache::getinstance().allocate(size);
}
void MyFree(void* ptr1)
{
	ThreadCache::getinstance().deallocate(ptr1);
}