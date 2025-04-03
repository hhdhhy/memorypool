#pragma once
#include "Memorypool.h"

class CentralCache
{
public:
    ~CentralCache()=default;
private:
    CentralCache();
    CentralCache& getinstance();


};

