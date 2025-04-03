#pragma once
#include "Memorypool.h"

class CentralCache
{
public:
    ~CentralCache()=default;
    CentralCache& getinstance();

    std::size_t fetch(void *&begin, void *&end, std::size_t idx, size_t num);

private:

    CentralCache();

    
    Span *get_span(size_t idx);

    Spanlist span_list_[LIST_SIZE];


};

