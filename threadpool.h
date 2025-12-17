#pragma once

#include <vector>
#include <thread>
#include <queue>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <future>
#include <stdexcept>

#include <type_traits>   
#include <utility>      
#include <memory>
#include <atomic>
#include <functional>
#include <iostream>
#include <deque>

class ThreadPool {
    public:
        explicit ThreadPool(size_t n);
        ~ThreadPool();

        template <typename F, typename... Args>

        auto submit(F&& f, Args&&... args)
            -> std::future<std::invoke_result_t<F, Args...>>;
        void wait_for_idle();

    private:
        void worker();
        struct WorkerState {
            std::deque<std::function<void()>> local;
            std::mutex m;
        };

        struct ActiveGuard {
            ThreadPool* pool;
            explicit ActiveGuard(ThreadPool* p) : pool(p) {
                pool->active_tasks.fetch_add(1, std::memory_order_relaxed);
            }
            ~ActiveGuard() {
                pool->active_tasks.fetch_sub(1, std::memory_order_relaxed);
                pool->outstanding.fetch_sub(1, std::memory_order_relaxed);
                pool->cv.notify_all();
            }
        };

        std::vector<std::thread> workers;
        std::vector<WorkerState> worker_states;

        std::deque<std::function<void()>> globalq;
        std::mutex global_mutex;

        std::mutex state_m;
        std::condition_variable cv;
        bool stop = false;
        
        std::atomic<size_t> active_tasks{0};
        std::atomic<size_t> outstanding{0};

        static thread_local int worker_id; // -1 if not a worker thread

};


template <typename F, typename... Args>

auto ThreadPool::submit(F&& f, Args&&... args) 
    -> std::future<std::invoke_result_t<F, Args...>> 
{
    using rtype = std::invoke_result_t<F, Args...>;

    auto task = std::make_shared<std::packaged_task<rtype()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...)
    );
    
    auto fut = task->get_future();

    // Check stop & increment outstanding
    {
        std::lock_guard<std::mutex> lk(state_m);
        if (stop) {
            throw std::runtime_error("Submit on stopped ThreadPool");
        }
        outstanding.fetch_add(1, std::memory_order_relaxed);
    }

    auto wrapper = [task]() { (*task)(); };

    if (worker_id >= 0) {
        // Submit to local queue
        WorkerState& w = worker_states[(size_t)worker_id];
        std::lock_guard<std::mutex> lk(w.m);
        w.local.push_back(std::move(wrapper));
    } else {
        // Submit to global queue
        std::lock_guard<std::mutex> lk(global_mutex);
        globalq.push_back(std::move(wrapper));
    }
    cv.notify_one();
    return fut;
}

