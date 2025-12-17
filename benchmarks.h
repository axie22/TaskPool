#pragma once

#include <iostream>
#include <chrono>
#include <vector>
#include <numeric>
#include <cassert>
#include "threadpool.h"

class benchmarks
{
private:
    /* data */
    static uint64_t heavy(uint64_t x);
public:
    benchmarks();
    ~benchmarks();
    void benchmark_p1();
    void benchmark_p2();
    void benchmark_p3();
    void benchmark_p4();
};