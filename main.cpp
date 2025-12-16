#include <iostream>
#include "threadpool.h"


int main() {
    ThreadPool pool(4);
    auto result = pool.submit([](int x) { return x * x; }, 5);
    std::cout << "Result: " << result.get() << std::endl;
    return 0;
}