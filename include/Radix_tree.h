#pragma once
#include<memory>
#include <cassert>
#include <cstring>
#include "Memorypool.h"
#include<iostream>
//三层基数树
class TCMalloc_PageMap3
{
private:
    static const std::size_t BITS = 64 - PAGE_SHIFT;
	static const std::size_t INTERIOR_BITS = (BITS + 2) / 3;       //第一、二层对应页号的比特位个数
	static const std::size_t INTERIOR_LENGTH = 1 << INTERIOR_BITS; //第一、二层存储元素的个数
	static const std::size_t LEAF_BITS = BITS - 2 * INTERIOR_BITS; //第三层对应页号的比特位个数
	static const std::size_t LEAF_LENGTH = 1 << LEAF_BITS;         //第三层存储元素的个数

	struct Leaf
	{
        Leaf()
		{
			memset(values, 0, sizeof(values));
		}
		Span* values[LEAF_LENGTH];
	};
    struct Node
	{
		Leaf* ptrs[INTERIOR_LENGTH];
	};

	Node* ptrs[INTERIOR_LENGTH];
public:
	typedef std::size_t Number;
    
    Span*& operator[](Number k)
    {
		assert(k<=SIZE_MAX);
		const Number i1 = k >> (LEAF_BITS + INTERIOR_BITS);         //第一层对应的下标
		const Number i2 = (k >> LEAF_BITS) & (INTERIOR_LENGTH - 1); //第二层对应的下标
		const Number i3 = k & (LEAF_LENGTH - 1);                    //第三层对应的下标
		// if(!Ensure(k))//确保映射第k页页号的空间是开辟好了的
        // return nullptr;
        // abort(); 
		if (ptrs[i1] == NULL) //第一层i1下标指向的空间未开辟
        {
            ptrs[i1]=new Node;
        }
        if (ptrs[i1]->ptrs[i2] == NULL) //第二层i2下标指向的空间未开辟
        {
            ptrs[i1]->ptrs[i2]=new Leaf;
        }
		return ptrs[i1]->ptrs[i2]->values[i3] ; 
	}
	//确保映射[start,start+n-1]页号的空间是开辟好了的
	bool Ensure(Number key)
	{
        const Number i1 = key >> (LEAF_BITS + INTERIOR_BITS);         //第一层对应的下标
        const Number i2 = (key >> LEAF_BITS) & (INTERIOR_LENGTH - 1); //第二层对应的下标
        if (i1 >= INTERIOR_LENGTH || i2 >= INTERIOR_LENGTH) //下标值超出范围
        return false;
        if (ptrs[i1] == NULL) //第一层i1下标指向的空间未开辟
        {
            ptrs[i1]=new Node;
        }
        if (ptrs[i1]->ptrs[i2] == NULL) //第二层i2下标指向的空间未开辟
        {
            ptrs[i1]->ptrs[i2]=new Leaf;
        }
		return true;
	}
    bool check(Number key)
	{
        const Number i1 = key >> (LEAF_BITS + INTERIOR_BITS);         //第一层对应的下标
        const Number i2 = (key >> LEAF_BITS) & (INTERIOR_LENGTH - 1); //第二层对应的下标
		const Number i3 = key & (LEAF_LENGTH - 1);
        if (i1 >= INTERIOR_LENGTH || 
            i2 >= INTERIOR_LENGTH||
			i3 >= LEAF_LENGTH||
            ptrs[i1] == NULL||
            ptrs[i1]->ptrs[i2] == NULL||
			ptrs[i1]->ptrs[i2]->values[i3] == NULL) //下标值超出范围||空间未开辟
        return false;
      
		return true;
	}

    void erase(Number key)
    {
        if(check(key))
        operator[](key)=NULL;
    }
};