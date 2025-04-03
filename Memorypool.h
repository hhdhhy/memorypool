#pragma once
#include <algorithm>
#include <mutex>

constexpr std::size_t ALIGNMENT = sizeof (std::size_t);// ALIGNMENT等于size_t的大小
constexpr std::size_t MAX_BYTES = 256 * 1024; // 256KB
//constexpr std::size_t FREE_LIST_SIZE = MAX_BYTES / ALIGNMENT; 
constexpr std::size_t LIST_SIZE = 220; 
constexpr std::size_t PAGE_MAX_NUM = 128;
constexpr std::size_t PAGE_SIZE = 4*1024;
constexpr std::size_t PAGE_SHIFT = 12;


size_t round_up(size_t size)
{
    if(size<=16)//2B
    {
        return  round_up_to(size,2);  //(16-0)/2=8
    }
    else if(size<=64)//4B
    {
        return  round_up_to(size,4); //(64-16)/4=12
    }
    else if(size<=128)//8B
    {
        return  round_up_to(size,8); //(128-64)/8=8
    }
    else if(size<=1024)//16B
    {
        return  round_up_to(size,16); //(1024-128)/16=56      
    }
    else if(size<=8192)//128B
    {
        return  round_up_to(size,128);//(8192-1024)/128=56
    }
    else if(size<=65536)//1024B
    {
        return  round_up_to(size,1024);//(65536-8192)/1024=56
    }
    else if(size<=262144)//8*1024B
    {
        return  round_up_to(size,8*1024);//(262144-65536)/(8*1024)=24
    }
    
    abort();
}
std::size_t get_idx(std::size_t size)
{
    if(size<=16)//2B
    {
        return  get_idx_to(size,2);  //(16-0)/2=8
    }
    else if(size<=64)//4B
    {
        return  get_idx_to(size-16,4)+8; //(64-16)/4=12
    }
    else if(size<=128)//8B
    {
        return  get_idx_to(size-64,8)+20; //(128-64)/8=8
    }
    else if(size<=1024)//16B
    {
        return  get_idx_to(size-128,16)+28; //(1024-128)/16=56      
    }
    else if(size<=8192)//128B
    {
        return  get_idx_to(size-1024,128)+84;//(8192-1024)/128=56
    }
    else if(size<=65536)//1024B
    {
        return  get_idx_to(size-8192,1024)+140;//(65536-8192)/1024=56
    }
    else if(size<=262144)//8*1024B
    {
        return  get_idx_to(size-65536,8*1024)+196;//(262144-65536)/(8*1024)=24
    }
    abort();
}
std::size_t get_idx_to(std::size_t size,size_t base)
{
    return (size+base-1)/base -1;
}

std::size_t round_up_to(std::size_t size,std::size_t base)//base为2的幂
{
    return (size + base - 1) & ~(base - 1);
}

inline void*& next(void* ptr)
{
    return *reinterpret_cast<void**>(ptr);
}

struct Span
{
    std::size_t Pid_;//pageid mmap给的内存是按页面大小对齐的 Pid_<<PAGE_SHIFT就是该内存的起始地址

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
        available_num_=0;
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
    std::size_t& get_available_num()
    {
        return available_num_;
    }
    Span*& get_head()
    {
        return head_;
    }
private:

    Span* head_;
    std::mutex mutex_;
    std::size_t available_num_; //可用的span数量
};