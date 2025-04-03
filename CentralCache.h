#pragma once
#include "Memorypool.h"
#include <unordered_map>
class CentralCache
{
public:
    ~CentralCache()=default;
    static CentralCache& getinstance();

    std::size_t get_page_num(std::size_t size);

    std::size_t fetch(void *&begin, void *&end, std::size_t size,std::size_t idx, size_t num);

private:

    CentralCache();

    
    Span *get_span(std::size_t size,std::size_t idx);

    Spanlist span_list_[LIST_SIZE];

};

