#pragma once
#include <algorithm>
#include <mutex>


constexpr std::size_t ALIGNMENT = sizeof (std::size_t);// ALIGNMENT等于size_t的大小
constexpr std::size_t MAX_BYTES = 256 * 1024; // 256KB
constexpr std::size_t FREE_LIST_SIZE = MAX_BYTES / ALIGNMENT; 
constexpr std::size_t LIST_SIZE = MAX_BYTES / ALIGNMENT; 
constexpr std::size_t PAGE_MAX_SIZE = 128;
std::size_t get_idx(std::size_t size)
{
    size= std::max(size, ALIGNMENT);
    return (size + ALIGNMENT - 1) / ALIGNMENT - 1;//向上取整

}
size_t roundUp(size_t size)
{
    return (size + ALIGNMENT - 1) & ~(ALIGNMENT - 1);
}

inline void*& next(void* ptr)
{
    return *reinterpret_cast<void**>(ptr);
}

struct Span
{
    std::size_t Pid_;

    Span* next_;
    Span* prev_;
    
    void * list_; //侵入式链表
    std::size_t num_; //free_list_中剩余的节点数

};


class Spanlist//双向链表
{
public:
    Spanlist()
    {
        head_ = nullptr;
    }

    void insert(Span* pos,Span* ptr)
    {
        if(ptr->next_)
        {
            pos->next_->prev_ = ptr;
        }
        ptr->next_ = pos->next_;

        pos->next_ = ptr;
        ptr->prev_ = pos;
    }
    void remove(Span* pos)
    {
        if(pos==head_)
        {
            head_ = pos->next_;
        }
        if(pos->next_)
        {
            pos->next_->prev_ = pos->prev_;
        }
        if(pos->prev_)
        {
            pos->prev_->next_ = pos->next_;
        }
    }

    std::mutex& get_mutex()
    {
        return mutex_;
    }
private:

    Span* head_;
    std::mutex mutex_;
};