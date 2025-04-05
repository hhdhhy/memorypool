#include "Memorypool.h"
#include <unordered_map>

class PageCache
{
public:
    ~PageCache()=default;
    static PageCache& getinstance();

    Span* get_span(std::size_t page_num);
    void giveback_span(Span *span);
    
    Span *system_alloc(std::size_t page_num);

    Span *ptr_to_span(void *ptr);

private:
    Span *split(Span *span, std::size_t page_num);
    Spanlist span_list_[PAGE_MAX_NUM];
    std::mutex mutex_;
    std::unordered_map<std::size_t,Span*> span_map_;//page_id映射span

};

