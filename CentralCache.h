#pragma once
#include "Memorypool.h"
class CentralCache
{
public:
    ~CentralCache()=default;
    static CentralCache& getinstance();

    std::size_t get_page_num(std::size_t size);

    

    std::size_t get_mem(void *&begin, std::size_t size, std::size_t idx, size_t num);
    void giveback_mem(void *ptr,std::size_t size,std::size_t idx);
private:

    CentralCache();

    Span *get_span(std::size_t size, std::size_t idx, std::unique_lock<std::mutex> &lock);

    Spanlist span_list_[LIST_SIZE];
    std::mutex list_mutex_[LIST_SIZE];
};

