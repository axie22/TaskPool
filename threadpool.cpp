#include "iostream"
#include "threadpool.h"
#include <future>
#include <queue>

ThreadPool::ThreadPool(size_t n) {
    for (size_t i = 0; i < n; ++i) {
        workers.emplace_back(&ThreadPool::worker, this);
    }
}

ThreadPool::~ThreadPool() {
    {
        std::unique_lock<std::mutex> lock(m);
        stop = true;
    }
    cv.notify_all();
    for (std::thread &worker : workers) {
        if (worker.joinable()) {
            worker.join();
        }
    }
}

void ThreadPool::worker() {
    while (true) {
        std::function<void()> job;
        {
            std::unique_lock<std::mutex> lock(m);
            cv.wait(lock, [this] { // we need [this] to know what stop and tasks are
                return stop || !tasks.empty();
            });
            if (stop && tasks.empty()) {
                return;
            }

            job = std::move(tasks.front());
            tasks.pop();
        }

        try {
            ActiveGuard guard(active_tasks);
            job();
        } catch (const std::exception& e) {
            std::cerr << "Exception in ThreadPool worker: " << e.what() << std::endl;
        } catch (...) {
            std::cerr << "Unknown exception in ThreadPool worker." << std::endl;
        }
    }
}
