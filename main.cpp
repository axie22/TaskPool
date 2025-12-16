#include <iostream>
#include <chrono>
#include <vector>
#include <numeric>
#include "threadpool.h"

static uint64_t heavy(uint64_t x) {
    for (int i = 0; i < 50'000'000; ++i) {
        x = x * 2862933555777941757ULL + 3037000493ULL;
    }
    return x;
}

int main() {
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
