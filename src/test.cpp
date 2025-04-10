#include "ThreadCache.h"
#include "iostream"
#include <vector>
#include <thread>
#include <atomic>
#include <random>
using namespace std;

void *ConcurrentAlloc(std::size_t size)
{
    return ThreadCache::getinstance().allocate(size);
}
void ConcurrentFree(void* ptr1)
{
	ThreadCache::getinstance().deallocate(ptr1);
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
	ConcurrentFree(v[i]);
	// void* ptr = ConcurrentAlloc(512);
	// cout << "-----" << ptr << endl;
}
void TestConcurrentFree1()
{
	void* p[500];
	int v[500];
	for(int i = 0; i < 500; ++i)
	{
		v[i]= rand() % MAX_BYTES + 1;
		ConcurrentAlloc(v[i]);
	}
	for(int i = 0; i < 500; ++i)
	{
		ConcurrentFree(p[i]);
	}
	
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
		ConcurrentFree(e);
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
		ConcurrentFree(v[i]);
	}
}
 
void TestMultiThread()
{
	std::thread t1(MultiThreadAlloc1);
	std::thread t2(MultiThreadAlloc2);

	t1.join();
	t2.join();
}


void BenchmarkMalloc(size_t ntimes, size_t nworks, size_t rounds) {
    std::vector<std::thread> vthread(nworks);
    std::atomic<size_t> malloc_costtime(0);
    std::atomic<size_t> free_costtime(0);

    // std::random_device rd;
    std::mt19937 gen;
	

    for (size_t k = 0; k < nworks; ++k) {
        vthread[k] = std::thread([&, k]() {
			gen.seed(11144);
			std::uniform_int_distribution<> dist_size(1, 16 * 1024);  // 随机内存块大小
			std::uniform_int_distribution<> dist_count(ntimes, ntimes);  // 随机内存分配次数
            std::vector<void*> v(ntimes);
			std::vector<int> p(ntimes);

            for (size_t j = 0; j < rounds; ++j) {
                size_t begin1 = clock();
                size_t local_ntimes = dist_count(gen);  // 随机决定每轮分配多少次内存
                for (size_t i = 0; i < local_ntimes; i++) {
                    size_t size = dist_size(gen);  // 随机内存块大小
                    v[i] = malloc(size);  // 使用 malloc 进行分配
                }
                size_t end1 = clock();

                size_t begin2 = clock();
                for (size_t i = 0; i < local_ntimes; i++) {
                    free(v[i]);  // 使用 free 进行释放
                }
                size_t end2 = clock();

                malloc_costtime += (end1 - begin1);
                free_costtime += (end2 - begin2);
            }
        });
    }

    for (auto& t : vthread) {
        t.join();
    }

    printf("%zu个线程并发执行%zu轮次，每轮次malloc %zu次: 花费：%zu ms\n",
        nworks, rounds, ntimes, malloc_costtime.load());

    printf("%zu个线程并发执行%zu轮次，每轮次free %zu次: 花费：%zu ms\n",
        nworks, rounds, ntimes, free_costtime.load());

    printf("%zu个线程并发malloc&free %zu次，总计花费：%zu ms\n",
        nworks, nworks * rounds * ntimes, malloc_costtime.load() + free_costtime.load());
}

// 比较 ConcurrentAlloc 性能的函数
void BenchmarkConcurrentMalloc(size_t ntimes, size_t nworks, size_t rounds) {
    std::vector<std::thread> vthread(nworks);
    std::atomic<size_t> malloc_costtime(0);
    std::atomic<size_t> free_costtime(0);

    // std::random_device rd;
    

    for (size_t k = 0; k < nworks; ++k) {
        vthread[k] = std::thread([&, k]() {
			std::mt19937 gen;
			gen.seed(11144);
			std::uniform_int_distribution<> dist_size(1,16 * 1024);  // 随机内存块大小
			std::uniform_int_distribution<> dist_count(ntimes, ntimes);  // 随机内存分配次数
            std::vector<void*> v(ntimes);
            std::vector<int> p(ntimes);

            for (size_t j = 0; j < rounds; ++j) {
                size_t begin1 = clock();
                size_t local_ntimes = dist_count(gen);  // 随机决定每轮分配多少次内存
                for (size_t i = 0; i < local_ntimes; i++) {
                    // p[i] = ;  // 随机内存块大小
                    v[i] = ConcurrentAlloc(dist_size(gen));  // 使用 ConcurrentAlloc 进行分配
                }
                size_t end1 = clock();

                size_t begin2 = clock();
                for (size_t i = 0; i < local_ntimes; i++) {
                    ConcurrentFree(v[i]);  // 使用 ConcurrentFree 进行释放
                }
                size_t end2 = clock();

                malloc_costtime += (end1 - begin1);
                free_costtime += (end2 - begin2);
            }
        });
    }

    for (auto& t : vthread) {
        t.join();
    }

    printf("%zu个线程并发执行%zu轮次，每轮次concurrent alloc %zu次: 花费：%zu ms\n",
        nworks, rounds, ntimes, malloc_costtime.load());

    printf("%zu个线程并发执行%zu轮次，每轮次concurrent dealloc %zu次: 花费：%zu ms\n",
        nworks, rounds, ntimes, free_costtime.load());

    printf("%zu个线程并发concurrent alloc&dealloc %zu次，总计花费：%zu ms\n",
        nworks, nworks * rounds * ntimes, malloc_costtime.load() + free_costtime.load());
}

int main() {
	TestMultiThread();
    // 配置测试参数
    size_t ntimes = 1000;  // 每轮分配内存的次数
    size_t nworks = 8;     // 线程数
    size_t rounds = 100;    // 轮次

    // 执行 malloc 测试
    printf("========= Benchmark Malloc =========\n");
    BenchmarkMalloc(ntimes, nworks, rounds);

    // 执行 ConcurrentAlloc 测试
    printf("========= Benchmark Concurrent Malloc =========\n");
    BenchmarkConcurrentMalloc(ntimes, nworks, rounds);

    return 0;
}

// ntimes 一轮申请和释放内存的次数
// rounds 轮次
// nwors表示创建多少个线程
// void BenchmarkMalloc(size_t ntimes, size_t nworks, size_t rounds)
// {
// 	std::vector<std::thread> vthread(nworks);
// 	std::atomic<size_t> malloc_costtime (0);
// 	std::atomic<size_t> free_costtime(0);

// 	for (size_t k = 0; k < nworks; ++k)
// 	{
// 		vthread[k] = std::thread([&]() {
// 			std::vector<void*> v(ntimes);
// 			std::vector<int> e(ntimes);
// 			for (size_t j = 0; j < rounds; ++j)
// 			{
// 				size_t begin1 = clock();
// 				for (size_t i = 0; i < ntimes; i++)
// 				{
// 					// v[i]=(malloc(16));
// 					// int p= rand() % MAX_BYTES + 1;
// 					v[i]=malloc((16 + i) % 8192 + 1);
// 				}
// 				size_t end1 = clock();

// 				size_t begin2 = clock();
// 				for (size_t i = 0; i < ntimes; i++)
// 				{
// 					free(v[i]);
// 				}
// 				size_t end2 = clock();
				

// 				malloc_costtime += (end1 - begin1);
// 				free_costtime += (end2 - begin2);
// 			}
// 			});
// 	}

// 	for (auto& t : vthread)
// 	{
// 		t.join();
// 	}

// 	printf("%zu个线程并发执行%zu轮次，每轮次malloc %zu次: 花费：%zu ms\n",
// 		nworks, rounds, ntimes, malloc_costtime.load());

// 	printf("%zu个线程并发执行%zu轮次，每轮次free %zu次: 花费：%zu ms\n",
// 		nworks, rounds, ntimes, free_costtime.load());

// 	printf("%zu个线程并发malloc&free %zu次，总计花费：%zu ms\n",
// 		nworks, nworks * rounds * ntimes, malloc_costtime.load() + free_costtime.load());
// }


// // 单轮次申请释放次数 线程数 轮次
// void BenchmarkConcurrentMalloc(size_t ntimes, size_t nworks, size_t rounds)
// {
// 	std::vector<std::thread> vthread(nworks);
// 	std::atomic<size_t> malloc_costtime (0);
// 	std::atomic<size_t> free_costtime (0);

// 	for (size_t k = 0; k < nworks; ++k)
// 	{
// 		vthread[k] = std::thread([&]() {
// 			std::vector<void*> v(ntimes);
// 			std::vector<int> e(ntimes);
			

// 			for (size_t j = 0; j < rounds; ++j)
// 			{
// 				size_t begin1 = clock();
// 				for (size_t i = 0; i < ntimes; i++)
// 				{
// 					// e[i]= rand() % MAX_BYTES + 1;
// 					// v[i]=(ConcurrentAlloc(16));
// 					v[i]=(ConcurrentAlloc((16 + i) % 8192 + 1));
// 				}
// 				size_t end1 = clock();

// 				size_t begin2 = clock();
// 				for (size_t i = 0; i < ntimes; i++)
// 				{
					
// 					ConcurrentFree(v[i],(16 + i) % 8192 + 1);
// 					// ConcurrentFree(v[i],16 );

// 				}
// 				size_t end2 = clock();
				

// 				malloc_costtime += (end1 - begin1);
// 				free_costtime += (end2 - begin2);
// 			}
// 			});
// 	}

// 	for (auto& t : vthread)
// 	{
// 		t.join();
// 	}

// 	printf("%zu个线程并发执行%zu轮次，每轮次concurrent alloc %zu次: 花费：%zu ms\n",
// 		nworks, rounds, ntimes, malloc_costtime.load());

// 	printf("%zu个线程并发执行%zu轮次，每轮次concurrent dealloc %zu次: 花费：%zu ms\n",
// 		nworks, rounds, ntimes, free_costtime.load());

// 	printf("%zu个线程并发concurrent alloc&dealloc %zu次，总计花费：%zu ms\n",
// 		nworks, nworks * rounds * ntimes, malloc_costtime.load() + free_costtime.load());
// }

// int main()
// {
//     // ConcurrentAllocTest2();
// 	// TestConcurrentFree1();
// 	// std::thread t(TestConcurrentFree1);
// 	// t.join();
// 	// TestMultiThread();
// 	std::size_t n = 1000;
	
// 	cout << "==========================================================" << endl;
// 	BenchmarkMalloc(n, 10, 100);
// 	cout << endl << endl;
	
// 	BenchmarkConcurrentMalloc(n, 10, 100); 
	
// 	cout << "==========================================================" << endl;
// 	return 0;
// }
// // int main()
// // {
// // 	// TestConcurrentFree1();
// // 	TestMultiThread();
// // }