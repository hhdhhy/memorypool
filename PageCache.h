#include "Memorypool.h"

class PageCache
{
public:
    
    ~PageCache()=default;
    static PageCache& getinstance();

    Span* getspan(std::size_t page_num);
private:
    PageCache();

    Spanlist spanlist_[PAGE_MAX_NUM];
    std::mutex mutex_;
};

