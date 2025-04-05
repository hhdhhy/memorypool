#pragma once
// 三级基数树（Radix Tree）
template <int BITS>
class TCMalloc_PageMap3 
{
private:
    // 每个内部层级应消耗的位数（向上取整）
    static const int INTERIOR_BITS = (BITS + 2) / 3;
    static const int INTERIOR_LENGTH = 1 << INTERIOR_BITS;

    // 叶子层级应消耗的位数
    static const int LEAF_BITS = BITS - 2 * INTERIOR_BITS;
    static const int LEAF_LENGTH = 1 << LEAF_BITS;

    // 内部节点结构
    struct Node 
    {
        Node* ptrs[INTERIOR_LENGTH];
    };

    // 叶子节点结构
    struct Leaf 
    {
        void* values[LEAF_LENGTH];
    };

    Node* root_;                          // 基数树根节点
    void* (*allocator_)(size_t);          // 内存分配器函数指针

    // 创建新节点（内部使用）
    Node* NewNode() 
    {
        Node* result = reinterpret_cast<Node*>((*allocator_)(sizeof(Node)));
        if (result != NULL) {
            memset(result, 0, sizeof(*result));
        }
        return result;
    }

public:
    typedef uintptr_t Number;

    // 构造函数：初始化根节点并设置分配器
    explicit TCMalloc_PageMap3(void* (*allocator)(size_t)) 
    {
        allocator_ = allocator;
        root_ = NewNode();
    }

    // 获取指定键对应的值
    void* get(Number k) const 
    {
        const Number i1 = k >> (LEAF_BITS + INTERIOR_BITS);
        const Number i2 = (k >> LEAF_BITS) & (INTERIOR_LENGTH - 1);
        const Number i3 = k & (LEAF_LENGTH - 1);
        if ((k >> BITS) > 0 ||
            root_->ptrs[i1] == NULL || root_->ptrs[i1]->ptrs[i2] == NULL) {
            return NULL;
        }
        return reinterpret_cast<Leaf*>(root_->ptrs[i1]->ptrs[i2])->values[i3];
    }

    // 设置指定键对应的值
    void set(Number k, void* v) 
    {
        ASSERT(k >> BITS == 0); // 确保键在有效范围内
        const Number i1 = k >> (LEAF_BITS + INTERIOR_BITS);
        const Number i2 = (k >> LEAF_BITS) & (INTERIOR_LENGTH - 1);
        const Number i3 = k & (LEAF_LENGTH - 1);
        reinterpret_cast<Leaf*>(root_->ptrs[i1]->ptrs[i2])->values[i3] = v;
    }

    // 确保从start开始的n个键对应的节点已分配
    bool Ensure(Number start, size_t n) 
    {
        for (Number key = start; key <= start + n - 1;) 
        {
            const Number i1 = key >> (LEAF_BITS + INTERIOR_BITS);
            const Number i2 = (key >> LEAF_BITS) & (INTERIOR_LENGTH - 1);

            // 检查索引是否超出范围
            if (i1 >= INTERIOR_LENGTH || i2 >= INTERIOR_LENGTH)
                return false;

            // 如果需要，创建第二级节点
            if (root_->ptrs[i1] == NULL) 
            {
                Node* n_node = NewNode();
                if (n_node == NULL) return false;
                root_->ptrs[i1] = n_node;
            }

            // 如果需要，创建叶子节点
            if (root_->ptrs[i1]->ptrs[i2] == NULL) 
            {
                Leaf* leaf = reinterpret_cast<Leaf*>((*allocator_)(sizeof(Leaf)));
                if (leaf == NULL) return false;
                memset(leaf, 0, sizeof(*leaf));
                root_->ptrs[i1]->ptrs[i2] = reinterpret_cast<Node*>(leaf);
            }

            // 将键推进到下一个叶子节点的起始位置
            key = ((key >> LEAF_BITS) + 1) << LEAF_BITS;
        }
        return true;
    }

    // 预分配更多内存（当前未实现）
    void PreallocateMoreMemory() 
    {
    }
};