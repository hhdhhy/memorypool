#include "Memorypool.h"
#include <unordered_map>
#include "Radix_tree.h"
#include <atomic>
class PageCache
{
public:
    ~PageCache()=default;
    static PageCache& getinstance();

    Span* get_span(std::size_t page_num);
    void giveback_span(Span *span);
    
    Span *system_alloc(std::size_t page_num);

    Span *ptr_to_span(void *ptr);

    Span *get_new_Span();

    void delete_Span(Span *span);


private:
    PageCache();
    Span *split(Span *span, std::size_t page_num);
    Spanlist span_list_[PAGE_MAX_NUM];
    std::mutex mutex_;
    Spanlist new_Span_;
    static PageCache* instance_;
    // std::unordered_map<std::size_t,Span*> span_map_;//page_id映射span
    TCMalloc_PageMap3 span_map_;//page_id映射span
};

