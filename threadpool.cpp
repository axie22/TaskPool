#include <iostream>
#include "threadpool.h"

thread_local int ThreadPool::worker_id = -1;

ThreadPool::ThreadPool(size_t n) : worker_states(n) {
    workers.reserve(n);
    for (size_t i = 0; i < n; ++i) {
        workers.emplace_back(&ThreadPool::worker, this, i);
    }
}

ThreadPool::~ThreadPool() {
    wait_for_idle();
    {
        std::lock_guard<std::mutex> lock(state_m);
        stop = true;
    }
    cv.notify_all();
    for (std::thread &worker : workers) {
        if (worker.joinable()) {
            worker.join();
        }
    }
}

void ThreadPool::worker(size_t id) {
    worker_id  = (int)id;
    while (true) {
        std::function<void()> job;
        // Try to get a job from the local queue
        {
            std::lock_guard<std::mutex> lk(worker_states[id].m);
            if (!worker_states[id].local.empty()) {
                job = std::move(worker_states[id].local.front());
                worker_states[id].local.pop_front();
            }
        }

        if (!job) {
            // Try to get a job from the global queue
            std::lock_guard<std::mutex> lk(global_mutex);
            if (!globalq.empty()) {
                job = std::move(globalq.front());
                globalq.pop_front();
            }
        }

        // if still nothing, wait
        if (!job) {
            std::unique_lock<std::mutex> lk(state_m);
            cv.wait(lk, [this]{
                return stop || outstanding.load(std::memory_order_relaxed) > 0;
            });
            if (stop && outstanding.load(std::memory_order_relaxed) == 0) {
                return;
            }
            continue;
        }

        try {
            ActiveGuard guard(this);
            job();
        } catch (...) {
            // error logging
        }
    }
}

void ThreadPool::wait_for_idle() {
    std::unique_lock<std::mutex> lock(state_m);
    cv.wait(lock, [this]() {
        return outstanding.load(std::memory_order_relaxed) == 0;
    });
}