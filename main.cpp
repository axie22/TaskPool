#include <iostream>
#include <chrono>
#include <vector>
#include <numeric>
#include <cassert>
#include "threadpool.h"

static uint64_t heavy(uint64_t x) {
    for (int i = 0; i < 50'000'000; ++i) {
        x = x * 2862933555777941757ULL + 3037000493ULL;
    }
    return x;
}

void benchmark_p1() {
    using clock = std::chrono::high_resolution_clock;

    std::cout << "ThreadPool Benchmark\n" << std::thread::hardware_concurrency() << " hardware threads detected.\n";

    const int N = 100; // number of tasks

    for (int i = 1; i <= 11; ++i) {
        std::cout << "Running with " << (i) << " threads:\n";
        ThreadPool pool(i);

        // warmup
        auto warm = pool.submit([]{ return 1; });
        warm.get();

        std::vector<std::future<uint64_t>> futs;
        futs.reserve(N);
        auto t1 = clock::now();
        for (int j = 0; j < N; ++j) {
            futs.emplace_back(pool.submit(heavy, (uint64_t)j));
        }
        auto r = std::accumulate(futs.begin(), futs.end(), uint64_t(0),
            [](uint64_t acc, std::future<uint64_t>& f) {
                return acc + f.get();
            });
        auto t2 = clock::now();
        std::chrono::duration<double, std::milli> ms = t2 - t1;
        std::cout << "Time: " << ms.count() << " ms\n";
        std::cout << "Throughput: " << (N / (ms.count() / 1000.0)) << " tasks/sec\n";
        std::cout << "Sum (ignore): " << r << "\n\n";
    }
}

void benchmark_p2() {
    ThreadPool pool(4);

    const int N = 10000;
    std::atomic<int> counter{0};

    for (int i = 0; i < N; ++i) {
        pool.submit([&counter] {
            counter.fetch_add(1, std::memory_order_relaxed);
        });
    }

    pool.wait_for_idle();  
    std::cout << "counter = " << counter.load() << "\n";
    assert(counter.load() == N);
}

int main() {
    std::cout << "Starting benchmark 1...\n";
    benchmark_p1();
    std::cout << "Benchmark 1 complete.\n";

    std::cout << "Starting benchmark 2...\n";
    benchmark_p2();
    std::cout << "Benchmark 2 complete.\n";

    return 0;
}
