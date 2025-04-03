#include "Memorypool.h"

class PageCache
{
public:
    ~PageCache()=default;
    static PageCache& getinstance();

    Span* get_span(std::size_t page_num);
    Span *system_alloc(std::size_t page_num);

private:
    PageCache();

    Spanlist span_list_[PAGE_MAX_NUM];
    std::mutex mutex_;
    
    std::unordered_map<std::size_t,Span*> span_map_;//page_id映射span

};

