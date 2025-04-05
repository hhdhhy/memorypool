#include "ThreadCache.h"
#include "iostream"
#include <vector>
#include <thread>
#include <atomic>
using namespace std;

void *ConcurrentAlloc(std::size_t size)
{
    return ThreadCache::getinstance().allocate(size);
}
void ConcurrentFree(void* ptr1,std::size_t size)
{
	ThreadCache::getinstance().deallocate(ptr1,size);
}
void ConcurrentAllocTest1()
{
	void* ptr1 = ConcurrentAlloc(5);
	void* ptr2 = ConcurrentAlloc(8);
	void* ptr3 = ConcurrentAlloc(4);
	void* ptr4 = ConcurrentAlloc(6);
	void* ptr5 = ConcurrentAlloc(3);

	cout << ptr1 << endl;
	cout << ptr2 << endl;
	cout << ptr3 << endl;
	cout << ptr4 << endl;
	cout << ptr5 << endl;
}
void ConcurrentAllocTest2()
{
	vector<void*> v;
	for (int i = 0; i < 10000; ++i)
	{
		void* ptr = ConcurrentAlloc(16);
		cout << ptr << endl;
		v.push_back(ptr);
	}
	for (int i = 0; i < 10000; ++i)
	ConcurrentFree(v[i],16 );
	// void* ptr = ConcurrentAlloc(512);
	// cout << "-----" << ptr << endl;
}
void TestConcurrentFree1()
{
	void* ptr1 = ConcurrentAlloc(5);
	void* ptr2 = ConcurrentAlloc(8);
	void* ptr3 = ConcurrentAlloc(4);
	void* ptr4 = ConcurrentAlloc(6);
	void* ptr5 = ConcurrentAlloc(3);
	void* ptr6 = ConcurrentAlloc(3);
	void* ptr7 = ConcurrentAlloc(3);

	ConcurrentFree(ptr1,5);
	ConcurrentFree(ptr2,8);
	ConcurrentFree(ptr3,4);
	ConcurrentFree(ptr4,6);
	ConcurrentFree(ptr5,3);
	ConcurrentFree(ptr6,3);
	ConcurrentFree(ptr7,3);
}

void MultiThreadAlloc1()
{
	std::vector<void*> v;
	for (std::size_t i = 0; i < 7; ++i)
	{
		void* ptr = ConcurrentAlloc(6);
		v.push_back(ptr);
	}

	for (auto e : v)
	{
		ConcurrentFree(e,6);
	}
}

void MultiThreadAlloc2()
{
	std::vector<void*> v;
	for (std::size_t i = 0; i < 7; ++i)
	{
		void* ptr = ConcurrentAlloc(16);
		v.push_back(ptr);
	}

	for (int i = 0; i < 7; ++i)
	{
		ConcurrentFree(v[i],16);
	}
}
 
void TestMultiThread()
{
	std::thread t1(MultiThreadAlloc1);
	std::thread t2(MultiThreadAlloc2);

	t1.join();
	t2.join();
}

/*这里测试的是让多线程申请ntimes*rounds次，比较malloc和刚写完的ConcurrentAlloc的效率*/

/*比较的时候分两种情况，
一种是申请ntimes*rounds次同一个块大小的空间，
一种是申请ntimes*rounds次不同的块大小的空间*/

/*下面的代码稍微过一眼就好*/


// ntimes 一轮申请和释放内存的次数
// rounds 轮次
// nwors表示创建多少个线程
void BenchmarkMalloc(std::size_t ntimes, std::size_t nworks, std::size_t rounds)
{
	std::vector<std::thread> vthread(nworks);
	std::atomic<std::size_t> malloc_costtime(0);
	std::atomic<std::size_t> free_costtime(0);

	for (std::size_t k = 0; k < nworks; ++k)
	{
		vthread[k] = std::thread([&, k]() {
			std::vector<void*> v;
			v.reserve(ntimes);

			for (std::size_t j = 0; j < rounds; ++j)
			{
				std::size_t begin1 = clock();
				for (std::size_t i = 0; i < ntimes; i++)
				{
					// v.push_back(malloc(16)); // 每一次申请同一个桶中的块
					v.push_back(malloc((16 + i) % 8192 + 1));// 每一次申请不同桶中的块
				}
				std::size_t end1 = clock();

				std::size_t begin2 = clock();
				for (std::size_t i = 0; i < ntimes; i++)
				{
					free(v[i]);
				}
				std::size_t end2 = clock();
				v.clear();

				malloc_costtime += (end1 - begin1);
				free_costtime += (end2 - begin2);
			}
			});
	}

	for (auto& t : vthread)
	{
		t.join();
	}

	printf("%u个线程并发执行%u轮次，每轮次malloc %u次: 花费：%u ms\n",
		nworks, rounds, ntimes, malloc_costtime.load());

	printf("%u个线程并发执行%u轮次，每轮次free %u次: 花费：%u ms\n",
		nworks, rounds, ntimes, free_costtime.load());

	printf("%u个线程并发malloc&free %u次，总计花费：%u ms\n",
		nworks, nworks * rounds * ntimes, malloc_costtime.load() + free_costtime.load());
}


// 								单轮次申请释放次数 线程数 轮次
void BenchmarkConcurrentMalloc(std::size_t ntimes, std::size_t nworks, std::size_t rounds)
{
	std::vector<std::thread> vthread(nworks);
	std::atomic<std::size_t> malloc_costtime(0);
	std::atomic<std::size_t> free_costtime(0);

	for (std::size_t k = 0; k < nworks; ++k)
	{
		vthread[k] = std::thread([&]() {
			std::vector<std::pair<void*,int>> v;
			v.reserve(ntimes);
			int p=k;
			for (std::size_t j = 0; j < rounds; ++j)
			{
				std::size_t begin1 = clock();
				for (std::size_t i = 0; i < ntimes; i++)
				{
					// v.push_back({ConcurrentAlloc(16),16});
					v.push_back({ConcurrentAlloc((16 + i) % 8192 + 1),(16 + i) % 8192 + 1});
				}
				std::size_t end1 = clock();

				std::size_t begin2 = clock();
				for (std::size_t i = 0; i < ntimes; i++)
				{
					ConcurrentFree(v[i].first,v[i].second);
				}
				std::size_t end2 = clock();
				v.clear();

				malloc_costtime += (end1 - begin1);
				free_costtime += (end2 - begin2);
			}
			});
	}

	for (auto& t : vthread)
	{
		t.join();
	}

	printf("%u个线程并发执行%u轮次，每轮次concurrent alloc %u次: 花费：%u ms\n",
		nworks, rounds, ntimes, malloc_costtime.load());

	printf("%u个线程并发执行%u轮次，每轮次concurrent dealloc %u次: 花费：%u ms\n",
		nworks, rounds, ntimes, free_costtime.load());

	printf("%u个线程并发concurrent alloc&dealloc %u次，总计花费：%u ms\n",
		nworks, nworks * rounds * ntimes, malloc_costtime.load() + free_costtime.load());
}

int main()
{
    // ConcurrentAllocTest2();

	std::size_t n = 1000;
	
	cout << "==========================================================" << endl;
	// 这里表示4个线程，每个线程申请10万次，总共申请40万次
	BenchmarkMalloc(n, 4, 10);
	cout << endl << endl;
	
	// 这里表示4个线程，每个线程申请10万次，总共申请40万次
	BenchmarkConcurrentMalloc(n, 4, 10); 
	
	cout << "==========================================================" << endl;
	return 0;
}
// int main()
// {
// 	// TestConcurrentFree1();
// 	TestMultiThread();
// }