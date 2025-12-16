#include <iostream>
#include <future>
#include <queue>


class ThreadPool {
public:
    ThreadPool(size_t n);
    ~ThreadPool();

private:
    void worker();   

    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;

    std::mutex m;
    std::condition_variable cv;
    bool stop = false;
};


// F is type of callable we pass in
// Args.. is a parameter pack
template <typename F, typename... Args>

// auto is the trailing return type used because return type depends on template parameters
// F&& is a forwarding reference because F is a deduced template type
// deduces the return type at compile time
auto submit(F&& f, Args&&... args) -> std::future<std::invoke_result_t<F, Args...>> {
    using rtype = std::invoke_result_t<F, Args...>;

    // create a packaged_task that wraps the callable and its arguments
    auto task = std::make_shared<std::packaged_task<rtype()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...)
    );
    
    // get the future associate with the result
    std::future<rtype> res = task->get_future();

    // brackets define scope for the lock
    {
        std::unique_lock<std::mutex> lock(m);
        if (stop) {
            throw std::runtime_error("submit on stopped ThreadPool");
        }

        tasks.emplace([task]() mutable {
            (*task)();
        });
    } // unlocks mutex here
    cv.notify_one();
    return res;
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
            job();
        } catch (const std::exception& e) {
            std::cerr << "Exception in ThreadPool worker: " << e.what() << std::endl;
        } catch (...) {
            std::cerr << "Unknown exception in ThreadPool worker." << std::endl;
        }
    }
}

int main() {
    return 0;
}