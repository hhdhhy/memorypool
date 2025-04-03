#include "Memorypool.h"

class PageCache
{
public:
    
    ~PageCache()=default;
    PageCache& getinstance();
    
private:
    PageCache();

    Spanlist spanlist_[PAGE_MAX_SIZE];
    std::mutex mutex_;
};

