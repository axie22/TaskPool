#include <iostream>
#include "benchmarks.h"

int main() {
    benchmarks bm;

    std::cout << "Starting benchmark 1...\n";
    bm.benchmark_p1();
    std::cout << "Benchmark 1 complete.\n";

    std::cout << "Starting benchmark 2...\n";
    bm.benchmark_p2();
    std::cout << "Benchmark 2 complete.\n";

    std::cout << "Starting benchmark 3...\n";
    bm.benchmark_p3();
    std::cout << "Benchmark 3 complete.\n";

    std::cout << "Starting benchmark 4...\n";
    bm.benchmark_p4();
    std::cout << "Benchmark 4 complete.\n";


    return 0;
}
